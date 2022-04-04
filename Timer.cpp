//
// Created by omri.tamam on 04/04/2022.
//

#include "Timer.h"

int Timer::Init(){
    struct sigaction sa = {0};
    struct itimerval timer;
    total_quantums = 1;

    sigemptyset(&sig_set);
    sigaddset (&sig_set, SIGINT);

    // Install timer_handler as the signal handler for SIGVTALRM.
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
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        printf("set timer error\n");
        return -1;
    }
    return 0;
}
