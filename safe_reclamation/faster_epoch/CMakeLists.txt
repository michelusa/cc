cmake_minimum_required(VERSION 3.16)
project(epoch_gc )

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE ( Threads REQUIRED )
add_executable(epoch
        faster_epoch.cc)
list(APPEND CXX_BASIC_FLAGS "-Wall;-Wextra;-Wshadow;-Wunused;-Wpedantic")
set(SAN_FLAGS "-fsanitize=address;-fno-omit-frame-pointer")
TARGET_COMPILE_OPTIONS(epoch PUBLIC ${CXX_BASIC_FLAGS} ${SAN_FLAGS})
TARGET_LINK_LIBRARIES (epoch PUBLIC Threads::Threads ${SAN_FLAGS})
