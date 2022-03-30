#ifndef OS_EX2_TERMINATOR_H
#define OS_EX2_TERMINATOR_H
#include "PoolManager.h"
#include "Starter.h"

class Terminator{

    private:
        PoolManager pool;
        Starter start;

    public:
    Terminator(PoolManager pool, Starter start);

};


#endif
