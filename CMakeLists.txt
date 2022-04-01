cmake_minimum_required(VERSION 3.17)
project(OS_ex2)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(OS_ex2
#        Demo/demo_jmp.c
        Demo/demo_itimer.c
        Thread.h
        PoolManager.h
        Terminator.h
        Timer.h
        Starter.h
        uthreads.h uthreads.cpp
        Thread.cpp Location.cpp Starter.cpp
        )
