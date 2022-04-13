
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
    struct itimerval timer;

    inline static sigset_t sig_set = {{0}};

 public:
    inline static int totalQuantum = 1;
    int init(PoolManager *staticPool);
    static void switchThread(int sig);

    int initTimer(int quantum_usecs){
        this->quantum_usecs = quantum_usecs;
        struct sigaction sa = {0};
        sigemptyset(&sig_set);
        sigaddset (&sig_set, SIGVTALRM);
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
        return launchTimer();
    }

    int launchTimer() const {
        mask_signals();
        if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
            printf("set timer error\n");
            return -1;
        }
        return 0;
    }

    static void mask_signals(){
        if(sigprocmask(SIG_BLOCK, &sig_set, nullptr)){
            fprintf(stderr, "system error: mask signals fail\n");
        }
    }

    static void unmask_signals(){
        if(sigprocmask(SIG_UNBLOCK, &sig_set, nullptr)){
            fprintf(stderr, "system error: mask signals fail\n");
        }
    }

};
#endif