

#define USEC_IN_SEC 1000000

#ifndef _TIMER_H
#define _TIMER_H
#include <sys/time.h>
#include <signal.h>
#include "Starter.h"

class Timer{

private:
    int quantum_usecs;
    sigset_t sig_set = {{0}};

public:
    int total_quantums;
    Timer(int quantum_usecs) {
        this->quantum_usecs = quantum_usecs;
        this->total_quantums = 1;
    }
    int Init();


};
#endif