#include <armadillo>
#ifndef OS_EX2_DISPATCHER_H
#include "Dispatcher.h"
#endif
int RETURN_FROM_STARTER = 5;
static Scheduler *pool;
struct itimerval timer{};
static sigset_t sig_set;



int Dispatcher::init(Scheduler *scheduler) {
    pool = scheduler;
    Dispatcher::totalQuantum = 1;
    return 0;
}

void Dispatcher::switchThread(int sig) {
    mask_signals();
    int ret_val = setjmp(pool->curRunning->env);
    if (ret_val == RETURN_FROM_STARTER)
    {
        unmask_signals();
        return;
    }
#pragma region handle old thread
    if(Scheduler::curRunning->status == RUNNING && (!Scheduler::curRunning->isSleep)){
        pool->preemptedThread();
    }
#pragma endregion
    //should check if there is no next available?
    Thread *nextThread = pool->nextAvailableReady();
    pool->setRunning(nextThread);
    Dispatcher::totalQuantum++;
    pool->updateWaitingTime();
    Scheduler::curRunning->quantum++;
    //TODO error
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);
}

int Dispatcher::initTimer(int quantum_usecs) {
    struct sigaction sa = {0};
    sigemptyset(&sig_set);
    sigaddset (&sig_set, SIGVTALRM);
    sa.sa_handler = &Dispatcher::switchThread;
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
    launchTimer();
    return 0;
}

void Dispatcher::launchTimer() const {
    mask_signals();
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        fprintf(stderr, "system error: set timer error\n");
        exit(1);
    }
}

Dispatcher::Dispatcher() {
    sig_set = {{0}};
    Dispatcher::totalQuantum = 1;
}

void Dispatcher::unmask_signals() {
    if(sigprocmask(SIG_UNBLOCK, &sig_set, nullptr)){
        fprintf(stderr, "system error: mask signals fail\n");
        exit(1);
    }
}

void Dispatcher::mask_signals() {
    if(sigprocmask(SIG_BLOCK, &sig_set, nullptr)){
        fprintf(stderr, "system error: mask signals fail\n");
        exit(1);
    }
}



