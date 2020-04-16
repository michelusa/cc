#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

struct nopadding {
    short a;
    short b;
};

/*
   on my laptop:
getconf LEVEL1_DCACHE_LINESIZE
64
cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size
64
*/
struct yespadding {
    short a;
    char padding[64 - sizeof(a)];
    short b;
};

nopadding nopadded;
yespadding yespadded;
const unsigned COUNT {10000000};
short write_a() {

    for (unsigned i = 0; i < COUNT; ++i)
    {
        nopadded.a++;
    }
    return nopadded.a;

}
short write_b() {
    for (unsigned i = 0; i < COUNT; ++i)
    {
        nopadded.b++;
    }
    return nopadded.b;

}

short write_apad() {

    for (unsigned i = 0; i < COUNT; ++i)
    {
        yespadded.a++;
    }
    return yespadded.a;

}
short write_bpad() {
    for (unsigned i = 0; i < COUNT; ++i)
    {
        yespadded.b++;
    }
    return yespadded.b;

}



auto main() -> int
{
    cout << "size of short           : " << sizeof(short) << endl;
    cout << "size of struct nopadding: " << sizeof(nopadding) << endl;
    cout << "size of struct yespadding: " << sizeof(yespadding) << endl;

    //measure with 2 shorts next to each other
    auto start = std::chrono::system_clock::now();
    thread t1 (write_a);
    thread t2 (write_b);
    t1.join(); t2.join();
    const chrono::duration<double> durationno = std::chrono::system_clock::now() - start;
    
    //measure with padding 
    start = std::chrono::system_clock::now();
    thread t3 (write_apad);
    thread t4 (write_bpad);
    t3.join(); t4.join();
    const chrono::duration<double> durationyes = std::chrono::system_clock::now() - start;
    cout << "duration nopad: " <<  std::chrono::duration_cast<std::chrono::milliseconds>(durationno).count() << " milliseconds." << endl;
    cout << "duration yespad: " <<  std::chrono::duration_cast<std::chrono::milliseconds>(durationyes).count() << " milliseconds." << endl;
}


