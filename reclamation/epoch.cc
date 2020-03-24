#include <sstream>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <string.h>

const unsigned N {1000000}; //count for display
const unsigned SZ {1024};//allocation size
std::atomic<char *> val_pointer;

std::vector<unsigned> read_epochs;
unsigned  current_epoch{1};
using namespace std::chrono_literals;

std::mutex count_mutex; //protect old versions   container
std::vector<std::pair <char *, unsigned> > old_versions;

//garbage collector
//if all readers have moved on, clean up past version (old allocations)
void gc() {
    std::lock_guard<std::mutex> guard(count_mutex);

    const auto old_versions_count = old_versions.size();

    if (!old_versions_count)
        return;
    const auto min_epoch = *std::min_element(read_epochs.begin(), read_epochs.end());
    ++current_epoch;

     for (unsigned  idx = 0; idx < old_versions.size(); ++idx) {
        if (old_versions[idx].second < min_epoch || min_epoch == std::numeric_limits<unsigned>::max()) {
            delete [] old_versions[idx].first;
            old_versions.erase(old_versions.begin() + idx);
        }
    }
    //debug if (old_versions_count > old_versions.size() )std::cout << "GC done old_areas old before: " << old_versions_count << " left: " <<old_versions.size() << std::endl;

}

//print summary message neatly
std::mutex log_mutex;
void message(const std::string& log) {
    std::lock_guard<std::mutex> guard(log_mutex);
    std::cout << "thread " << std::this_thread::get_id() << ":      " <<    log << std::endl;
}
bool halt {false};

//forever get pointer to allocated memory then display a byte from allocated memory as seen
//one reader call GC every so many
//unlikely to read each version, and can read more than once the same(!)
void read_it(unsigned core_idx) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_idx, &cpuset);
    if(0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        std::cerr << "affinity problem\n";return;
    }
    unsigned  iter{0};
    for(;;){
        ++iter;
        read_epochs[core_idx] = current_epoch;
        char *my_memory = val_pointer.load(std::memory_order_relaxed);

        //debug if ( core_idx==1)message(std::to_string(*my_memory));

        //call gc once in a while. this blocks writer. call cpu-1 times
        if ( iter % 720 == 0) {
            gc();
        }
        read_epochs[core_idx] = std::numeric_limits<unsigned>::max();

        //stop read after all write is done.
        if (halt)
            break;
    }

        gc();

    std::stringstream ss;
    ss << "read " << iter << " times" << std::endl;
    message(ss.str());

}
char * initial = new char [SZ];

//one writer exactly
void write_it() {

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    const unsigned core_idx_zero {0};
    CPU_SET(core_idx_zero, &cpuset);
    if(0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
        std::cerr << "affinity problem\n";return;
    }

    char *new_val = initial;

    size_t max_pressure {0};
    unsigned iter {0};

    std::this_thread::sleep_for(1s);
    auto start = std::chrono::system_clock::now();
    while(iter < N) {
        ++iter;

        char *old_val = val_pointer.load();
        new_val = new char [SZ];
        *new_val = iter % 127;

       val_pointer.store(new_val);

        std::lock_guard<std::mutex> guard(count_mutex);
        old_versions.emplace_back(old_val, current_epoch);

        max_pressure = std::max(max_pressure, old_versions.size());
    }
    const std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
    halt = true;
    std::stringstream ss;
    ss << "wrote " <<  iter << " times in " <<     std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " milliseconds.";
    ss << " max pressure: " << max_pressure << std::endl;
    message(ss.str());
}

//one reading thread per core thread (minus one for writer)
auto main() -> int
{
    const auto n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";
    if (n < 2) {
        std::cout << "not running" << std::endl;
        return 1;
    }
    std::cout << "initial: " <<  static_cast <const void *>( initial) << std::endl;
    val_pointer.store(initial);
    std::vector<std::thread> workers;
    std::thread writer (write_it);
    for (unsigned  idx = 1; idx < n; ++idx) {
        read_epochs.push_back(std::numeric_limits<unsigned>::max());
        workers.emplace_back(read_it, idx);
    }
    for (auto& t : workers)
        t.join();
    writer.join();

}

