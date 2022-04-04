#include <sys/time.h>
#include <signal.h>
#include "Starter.h"

#define USEC_IN_SEC 1000000

#ifndef _TIMER_H
#define _TIMER_H

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

    void mask_signals(bool mask){
        if(mask){
            if(sigprocmask(SIG_BLOCK, &sig_set, nullptr) == -1){
            //print sigerr
            }
        }
        else{
            if(sigprocmask(SIG_UNBLOCK, &sig_set, nullptr) == -1){
                //print sigerr
            }
        }
    }
};
#endif