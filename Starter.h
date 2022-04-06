
#ifndef OS_EX2_STARTER_H
#define OS_EX2_STARTER_H

#include <sys/time.h>
#include <signal.h>
#include "PoolManager.h"
#define USEC_IN_SEC 1000000

class Starter {

 private:
    inline static PoolManager *pool;
    int quantum_usecs;
    inline static sigset_t sig_set = {{0}};
 public:
    inline static int totalQuantum;
    int init(PoolManager *staticPool);
    static void switchThread(int sig);
    int initTimer(int quantum_usecs){
        struct sigaction sa = {0};
        struct itimerval timer;
        totalQuantum = 1;
        sigemptyset(&sig_set);
        sigaddset (&sig_set, SIGINT);
        sa.sa_handler = &Starter::switchThread;
        if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
            printf("seg error\n");
            return -1;
        }

        int sec = quantum_usecs / USEC_IN_SEC;
        int usec =  quantum_usecs % USEC_IN_SEC;

        // Configure the timer to expire after 1 usec... */
        timer.it_value.tv_sec = sec;		// first time interval, seconds part
        timer.it_value.tv_usec = usec;		// first time interval, microseconds part

        // configure the timer to expire every quantum_usecs after that.
        timer.it_interval.tv_sec = sec;		                // first time interval, seconds part
        timer.it_interval.tv_usec = usec;		// first time interval, microseconds part


        // Start a virtual timer. It counts down whenever this process is executing.
        Starter::mask_signals(true);
        if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
            printf("set timer error\n");
            return -1;
        }
        return 0;
    }
    static void mask_signals(bool mask){
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

    static void log(const char *prefix);
};
#endif