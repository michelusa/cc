#include <iostream>
#include <chrono>
constexpr int u2bin (const auto val) {
    return val?
        (val & 1) + (10 * u2bin (val >> 1))
        :
        0;

}

constexpr long fibi_compile_time (const unsigned f) {
    return (f <= 1) ? f : fibi_compile_time(f-1) + fibi_compile_time(f-2);
}

long fibi_run_time (const unsigned f) {
    return (f <= 1) ? f : fibi_run_time(f-1) + fibi_run_time(f-2);
}


auto main() -> int {
    std::cout << u2bin(257) << std::endl;


    auto start = std::chrono::system_clock::now();
    const auto X =  fibi_compile_time (30) ;
    std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
    std::cout << "constexpr duration(not including compile time!): " <<  std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microseconds. result = "<< X << std::endl;

    start = std::chrono::system_clock::now();
    const auto Y =  fibi_run_time (30) ;
    duration = std::chrono::system_clock::now() - start;
    std::cout << "RT recurse duration: " <<  std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microseconds. result = " << Y <<std::endl;




}
