cmake_minimum_required(VERSION 3.20)
project(OS_ex2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(OS_ex2
        Demo/demo_jmp.c
#        Demo/demo_itimer.c
        #uthreads.h uthreads.cpp
        PoolManager.cpp Thread.cpp Thread.h Location.cpp Starter.cpp PoolManager.h Terminator.cpp Terminator.h Starter.h Timer.cpp Timer.h)
