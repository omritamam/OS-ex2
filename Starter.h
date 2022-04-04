
#ifndef OS_EX2_STARTER_H
#define OS_EX2_STARTER_H

#include "PoolManager.h"

int RETURN_FROM_STARTER = 5;

class Starter {

 private:
    static PoolManager *pool;
 public:
    static int totalQuantum;
    int init(PoolManager *staticPool);
    static void switchThread(int sig);
};
#endif