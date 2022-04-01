#include <sys/time.h>
#include "Terminator.h"
#define USEC_IN_SEC 1000000

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
    int Init(){
        struct sigaction sa = {0};
        struct itimerval timer;
        total_quantums = 1;

        sigemptyset(&sig_set);
        sigaddset (&sig_set, SIGINT);

        // Install timer_handler as the signal handler for SIGVTALRM.
        sa.sa_handler = &Terminator::terminate;
        if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
            printf("seg error\n");
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
        if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
            printf("set timer error\n");
        }

    }
};