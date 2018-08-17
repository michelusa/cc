#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

const unsigned N = 10000000;
std::atomic<unsigned> counter;
std::mutex count_mutex;
unsigned simple_counter;


void mutex_op(const unsigned wall, unsigned core_idx, const bool micro)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_idx, &cpuset);
  if(0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
    std::cerr << "affinity problem\n";return;
  }

  if(!micro) {
    std::lock_guard<std::mutex> guard(count_mutex);
    for(unsigned i = 0; i < wall; ++i) {
      simple_counter++;
    }
  } else {

    for(unsigned i = 0; i < wall; ++i) {
      std::lock_guard<std::mutex> guard(count_mutex);
      simple_counter++;
    }
  }
}
void op(const unsigned wall, unsigned core_idx, std::memory_order mem_order)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_idx, &cpuset);
  if(0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) {
    std::cerr << "affinity problem\n";return;
  }

  for(unsigned i = 0; i < wall; ++i) {
    counter.fetch_add(1, mem_order);
  }
}


void test(const unsigned n, const std::string& label, std::memory_order mem_order)
{
  std::vector<std::thread> workers;
  counter = 0;

  auto start = std::chrono::system_clock::now();
  for(unsigned core_idx = 0; core_idx < n; ++core_idx) {
    //assign each thread to a core
    workers.emplace_back(op, N, core_idx, mem_order);
  }
  for(auto& t : workers) t.join();
  const std::chrono::duration<double> dtion = std::chrono::system_clock::now() - start;

  std::cout << label << " result: " << counter;
  std::cout << " in " << std::chrono::duration_cast<std::chrono::microseconds>(dtion).count();
  std::cout << " us.\n";
}


void mutex_test(const unsigned n, const std::string& label, const bool micro)
{
  std::vector<std::thread> workers;
  counter = 0;

  auto start = std::chrono::system_clock::now();
  for(unsigned core_idx = 0; core_idx < n; ++core_idx) {
    //assign each thread to a core
    workers.emplace_back(mutex_op, N, core_idx, micro);
  }
  for(auto& t : workers) t.join();
  const std::chrono::duration<double> dtion = std::chrono::system_clock::now() - start;

  std::cout << label << " result: " << simple_counter;
  std::cout << " in " << std::chrono::duration_cast<std::chrono::microseconds>(dtion).count();
  std::cout << " us.\n";
}


auto main() -> int
{
  const auto n = std::thread::hardware_concurrency();
  std::cout << n << " concurrent threads are supported.\n";
  test(n, "seq cst", std::memory_order_seq_cst);
  test(n, "relax", std::memory_order_relaxed);
  mutex_test(n, "mutex macro", false);
  mutex_test(n, "mutex micro", true);
}

