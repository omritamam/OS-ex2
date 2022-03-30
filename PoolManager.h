//
// Created by shirayarhi on 30/03/2022.
//
#include "Location.cpp"
#include <vector>
#include "Thread.h"

#ifndef OS_EX2_POOLMANAGER_H
#define OS_EX2_POOLMANAGER_H
class PoolManager {
    //fields:
private:
    std::vector<Thread> ready;
    std::vector<Thread> blocked;

public:
    Thread running;

    //Methods
    PoolManager();
    int Move(Location scr, Location des);
    Thread nextAvailableReady();
    int GetId();

};#endif
