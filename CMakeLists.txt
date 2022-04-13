cmake_minimum_required(VERSION 3.17)
project(OS_ex2)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(OS_ex2
        Dispather.cpp
        Scheduler.cpp
        uthreads.cpp
#
        )
