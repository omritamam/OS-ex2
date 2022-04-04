cmake_minimum_required(VERSION 3.17)
project(OS_ex2)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(OS_ex2
#        Demo/demo_jmp.c
#        Demo/demo_itimer.c
        Thread.h
        uthreads.h

        PoolManager.h
        Starter.h
        uthreads.cpp
        Timer.cpp Starter.cpp PoolManager.cpp Utils.cpp Utils.h)
