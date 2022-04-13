
#ifndef OS_EX2_DISPATCHER_H
#define OS_EX2_DISPATCHER_H

#include <sys/time.h>
#include <signal.h>
#include "Scheduler.h"
#define USEC_IN_SEC 1000000


class Dispatcher {

 private:

 public:
    static int totalQuantum;
    int init(Scheduler *scheduler);
    static void switchThread(int sig);
    int initTimer(int quantum_usecs);
    void launchTimer() const;
    Dispatcher();

    static void mask_signals();

    static void unmask_signals();

};
#endif