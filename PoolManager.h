//
// Created by shirayarhi on 30/03/2022.
//
#include "Location.cpp"
#include <vector>
#include "Thread.h"
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

};
