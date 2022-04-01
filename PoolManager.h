
#include "Location.cpp"
#include "Thread.h"
class PoolManager {
    //fields:
private:

public:
    Thread running;

    //Methods
    PoolManager(){}
    int Move(Location scr, Location des);
    Thread nextAvailableReady();
    int GetId();

};


