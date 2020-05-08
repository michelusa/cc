#include <sstream>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>    //for logging summary
#include <algorithm>
#include <string.h>

/*
 * single writer on its own core,
 *  multiple readers each on their own core
 *  memory allocated and reclaim by writer
 */

const unsigned N{1000000}; //count of tries
const unsigned SZ{2048};//allocation size
static const auto cores = std::thread::hardware_concurrency();

char * val_pointer;    //pointer to memory allocated, shared, reclaimed

std::atomic<unsigned> current_epoch{0};

bool start {false};   //workers can start reading if true


void read_it(unsigned core_idx);
struct worker_record{
        std::thread t;
        unsigned epoch_na{0};
        worker_record(int idx) {
                t = std::move(std::thread(read_it,idx));
        }
};
worker_record * workers[3];

using namespace std::chrono_literals;

/*
 * verbose console feedback
 */
using std::cout;
using std::cerr;
using std::endl;
using namespace std::chrono;

std::mutex log_mutex;
void message(const std::string &log) {
        std::lock_guard<std::mutex> guard(log_mutex);
        cout << "thread " << std::this_thread::get_id();
        cout << ":      " << log << endl;
}

void client_summary(unsigned iter) {
        std::stringstream ss;
        ss << "read " << iter << " times" << std::endl;
        message(ss.str());
}

void writer_summary(unsigned iter, long duration) {

        std::stringstream ss;
        ss << "allocated and freed " << iter << " times in ";
        ss << duration << " milliseconds." <<  endl;
        message(ss.str());
}

/*
 * set thread on its core
 */
int set_affinity (unsigned core_idx) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_idx, &cpuset);
        const auto res =  pthread_setaffinity_np(pthread_self(),
                        sizeof(cpu_set_t),
                        &cpuset);
        if (res)
                cerr << "affinity problem\n";
        return res;
}

void read_it(unsigned core_idx) {
        if (set_affinity(core_idx))
                return;

        unsigned iter{0};

        //wait for publisher to start
        while (!start)
                std::this_thread::sleep_for(1s);

        for (;;) {


                auto x = *val_pointer;//PSEUDO WORK HERE
                ++iter;

                workers[core_idx-1]->epoch_na =  current_epoch.load();

                //wait for something new to read
                while (workers[core_idx-1]->epoch_na ==  current_epoch.load())
                        ;
                if (iter == N)
                        break;
        }

        client_summary(iter);
}

//one writer exactly
void write_it() {
        if (set_affinity(0))
                return;

        unsigned iter{0};

        const auto start_time = system_clock::now();

        while (iter < N) {
                ++iter;

                val_pointer = new char[SZ];
                *val_pointer = iter % 127;

                ++current_epoch;//atomic

                //all readers must catch up to  epoch
                for (unsigned i = 0; i < cores - 1; ++i) {
                        while (workers[i]->epoch_na < current_epoch.load())
                                ;
                }

                delete [] val_pointer;

                //finish the test
                if (iter == N){
                        ++current_epoch;
                        break;
                }

        }


        const duration<double> duration = system_clock::now() - start_time;

        writer_summary(iter, duration_cast<milliseconds>(duration).count());
}

auto main() -> int {
        std::cout << cores << " concurrent threads are supported.\n";
        if (cores < 2) {
                std::cout << "not enough cores to run" << std::endl;
                return 1;
        }

        for (unsigned idx = 1; idx < cores; ++idx) {
                workers[idx - 1] = new worker_record(idx );
                cout << "created worker: "<< (idx - 1) << endl;
        }

        std::thread writer(write_it);
        cout << "created producer" << endl;

        //give time to producer to alloc one before reading
        std::this_thread::sleep_for(2s);
        start = true;

        for (unsigned idx = 1; idx < cores; ++idx) {
                workers[idx - 1]->t.join();
                delete workers[idx - 1];
        }
        writer.join();

}

