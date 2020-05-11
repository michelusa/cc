cmake_minimum_required(VERSION 3.16)
project(epoch_gc )

set(CMAKE_CXX_STANDARD 17)
add_library(pthread SHARED IMPORTED)
FIND_PACKAGE ( Threads REQUIRED )
add_executable(epoch
        faster_epoch.cc)
TARGET_LINK_LIBRARIES ( epoch ${CMAKE_THREAD_LIBS_INIT} )
