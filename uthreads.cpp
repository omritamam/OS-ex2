#include <cstdlib>
#include <armadillo>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <setjmp.h>
#include <vector>
#include <set>
#include <queue>
#include "uthreads.h"
using namespace std;


// Objects
class Scheduler;
class Dispatcher;
class Thread;

//global variables
static Scheduler* scheduler;
static Dispatcher* dispatcher;
static Thread* curRunning;
static int totalQuantum = 1;
static struct itimerval timer;
static sigset_t sig_set;


int RETURN_FROM_STARTER = 5;
#define MANUAL_SWITCH 4
#define USEC_IN_SEC 1000000


// headers
class Dispatcher {

public:
    int init();
    static void switchThread(int sig);
    int initTimer(int quantum_usecs);
    int launchTimer() const;

    static void mask_signals();
};


class Thread {

public:
    int id;
    int status;
    sigjmp_buf env;
    int duplicate;
    int quantum;
    char* stackPointer;
    bool isSleep;
    int waitingTime;
};


#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define MAIN_THREAD_TID 0
#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define STACK_SIZE 4096 /* stack size per thread (in bytes) */


class Scheduler {

public:
    set<Thread *> *blockedID;
    std::deque<Thread *> *readyQueue;
    Thread* allThreads[MAX_THREAD_NUM] = {0};
    int countReady;

    Scheduler ()
    {
        countReady = 1;
        curRunning = nullptr;
        blockedID = new set<Thread *> ();
        readyQueue = new deque< Thread *> ();
    }
    void initMainThread ()
    {
        Thread *mainThread = new Thread ();
        mainThread->stackPointer = new char(1);
        mainThread->id = MAIN_THREAD_TID;
        mainThread->status = RUNNING;
        mainThread->duplicate = 0;
        mainThread->quantum = 1;
        mainThread->waitingTime = -1;
        mainThread->isSleep = false;
        sigemptyset (&(mainThread->env)->__saved_mask);
        allThreads[mainThread->id] =  mainThread;
        curRunning = mainThread;
        curRunning = mainThread;
        sigsetjmp(mainThread->env, 1);
    }
    int findId();
    int addThread (char *stack, thread_entry_point entry_point);
    Thread *nextAvailableReady ()
    {
        if(readyQueue->empty()){
            return allThreads[0];
        }
        Thread *candidate = readyQueue->front();
        readyQueue->pop_front();
        candidate->duplicate--;
        while ((candidate->status != READY) || (candidate->duplicate == 1))
        {

            if(readyQueue->empty()){
                return curRunning;
            }
            candidate = readyQueue->front ();
            readyQueue->pop_front();
            candidate->duplicate--;
        }
        return candidate;
    }
    int blockThread (int tid)
    {
        Thread* thread = allThreads[tid];
        thread->status = BLOCKED;
        auto ret = blockedID->insert(thread);
        return ret.second;
    }
    void preemptedThread ()
    {
        curRunning->status = READY;
        readyQueue->push_back(curRunning);
        curRunning->duplicate++;
    }
    int resumeThread (int tid)
    {
        Thread* thread = allThreads[tid];
        if (thread->status == BLOCKED)
        {
            blockedID->erase(thread);
            thread->status = READY;
            if(!thread->isSleep){
                readyQueue->push_back(thread);
                thread->duplicate++;
            }
            return 0;
        }
        return -1;
    }
    void terminateProcess ()
    {
        for (auto & thread : allThreads)
        {
            if(thread != nullptr)
                delete thread->stackPointer;
            delete thread;
        }
        allThreads[MAX_THREAD_NUM] = {0};

        delete blockedID;
        delete readyQueue;
    }
    int terminateThread (int tid)
    {
        Thread* thread = allThreads[tid];
        if(thread == nullptr){
            return -1;
        }
        readyQueue->erase(remove(readyQueue->begin(), readyQueue->end(), thread), readyQueue->end());
        allThreads[tid] = nullptr;
        delete thread->stackPointer;
        delete thread;
        countReady--;

        return 0;
    }
    static void setRunning (Thread * thread);
    unsigned long count(){
        return countReady;
    }
    void updateWaitingTime(){
        for (auto & thread : allThreads)
        {
            if(thread != nullptr){
                if(thread->isSleep){
                    thread->waitingTime--;
                }
                if(thread->waitingTime == 0){
                    thread->isSleep = false;
                    if(thread->status == READY){
                        readyQueue->push_back(thread);
                        thread->duplicate++;
                    }
                    thread->waitingTime = -1;
                }
            }
        }
    }
};


int Scheduler::addThread(char *stack, thread_entry_point entry_point) {
    Thread *newTread = new Thread ();
    newTread->stackPointer = stack;
    newTread->id = findId();
    newTread->status = READY;
    newTread->waitingTime = -1;
    newTread->isSleep = false;
    address_t sp = (address_t) stack + STACK_SIZE - sizeof (address_t);
    address_t pc = (address_t) entry_point;
    sigsetjmp(newTread->env, 1);
    ((newTread->env)->__jmpbuf)[JB_SP] = translate_address (sp);
    ((newTread->env)->__jmpbuf)[JB_PC] = translate_address (pc);
    sigemptyset (&(newTread->env)->__saved_mask);
    countReady++;
    allThreads[newTread->id]  = newTread;
    readyQueue->push_back(newTread);
    newTread->duplicate = 1;
    return newTread->id;
}

void Scheduler::setRunning(Thread *thread) {
    thread->status = RUNNING;
    curRunning = thread;
}

int Scheduler::findId() {
    for(int i = 1; i < MAX_THREAD_NUM; i++){
        if(allThreads[i] == nullptr){
            return i;
        }
    }
    return -1;
}


int Dispatcher::initTimer(int quantum_usecs) {
    struct sigaction sa = {0};
    sigemptyset(&sig_set);
    sigaddset (&sig_set, SIGVTALRM);
    sa.sa_handler = &Dispatcher::switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        fprintf(stderr,"system error: failed set timer\n");
        exit(1);
    }

    int sec = quantum_usecs / USEC_IN_SEC;
    int usec =  quantum_usecs % USEC_IN_SEC;

    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = usec;
    timer.it_interval.tv_sec = sec;
    timer.it_interval.tv_usec = usec;
    launchTimer();
    return 0;
}

int Dispatcher::launchTimer() const {
    mask_signals();
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        printf("set timer error\n");
        return -1;
    }
    return 0;
}

static void mask_signals() {
    if(sigprocmask(SIG_BLOCK, &sig_set, nullptr)){
        fprintf(stderr, "system error: mask signals fail\n");
    }
}

void unmask_signals() {
    if(sigprocmask(SIG_UNBLOCK, &sig_set, nullptr)){
        fprintf(stderr, "system error: mask signals fail\n");
    }
}

int Dispatcher::init() {
    totalQuantum = 1;
    return 0;
}

void Dispatcher::switchThread(int sig) {
    mask_signals();
    int ret_val = setjmp(curRunning->env);
    if (ret_val == RETURN_FROM_STARTER){
        unmask_signals();
        return;
    }
    if(curRunning->status == RUNNING && (!curRunning->isSleep)){
        scheduler->preemptedThread();
    }
    Thread *nextThread = scheduler->nextAvailableReady();
    scheduler->setRunning(nextThread);
    totalQuantum++;
    scheduler->updateWaitingTime();
    curRunning->quantum++;
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);
}

void Dispatcher::mask_signals(){
    if(sigprocmask(SIG_BLOCK, &sig_set, nullptr)){
        fprintf(stderr, "system error: mask signals fail\n");
    }
}

int checkValidTid(int tid){
    if ( tid < 0 || tid > 99 )
    {
        fprintf(stderr, "thread library error: the thread id is invalid (it needs to be  between 0 to 99)\n");
        return -1;
    }
    if ( scheduler->allThreads[tid] == nullptr)
    {
        fprintf(stderr, "thread library error: the thread id's cell is empty in the threadsArr\n");
        return -1;
    }
    return 0;
}


int uthread_terminate (int tid)
{
    mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    { //main thread terminated
        scheduler->terminateProcess ();
        delete scheduler;
        delete dispatcher;
        exit(0);
    }
    if (curRunning->id == tid)
    { //terminate himself
        if (scheduler->terminateThread(tid))
        {
            return -1;
        }
        dispatcher->launchTimer();
        dispatcher->switchThread (0);
    }
    if (scheduler->terminateThread (tid))
    {
        return -1;
    }
    unmask_signals();
    return 0;
}

int uthread_block(int tid)
{
    mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    {
        fprintf(stderr, "thread library error: it's illegal to block the main thread\n");
        return -1;
    }
    scheduler->blockThread(tid);
    if (curRunning->id == tid)
    {
        //block himself
        dispatcher->launchTimer();
        dispatcher->switchThread (MANUAL_SWITCH);
    }
    unmask_signals();
    return 0;
}


int uthread_resume(int tid)
{
    mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if(!scheduler->allThreads[tid]->isSleep){
        scheduler->resumeThread(tid);
    }
    scheduler->allThreads[tid]->status = READY;
    unmask_signals();
    return 0;
};


int uthread_get_tid(){
    return curRunning->id;
}

int uthread_get_total_quantums(){
    return totalQuantum;
}

int uthread_get_quantums(int tid){
    if(checkValidTid(tid) == -1){
        return -1;
    }
    return scheduler->allThreads[tid]->quantum;
}

int uthread_init (int quantum_usecs)
{
    dispatcher = new Dispatcher();
    scheduler = new Scheduler();
    dispatcher->init();

    if (quantum_usecs <= 0)
    {
        fprintf (stderr, "thread library error: quantum_usecs must be positive\n");
        return -1;
    }
    dispatcher->initTimer(quantum_usecs);
    scheduler->initMainThread();
    unmask_signals();
    return 0;
}

int uthread_spawn (thread_entry_point entry_point)
{
    if(scheduler->count()>=MAX_THREAD_NUM){
        fprintf(stderr, "thread library error: reached the max number\n");
        return -1;
    }
    if(entry_point == nullptr){
        fprintf(stderr, "thread library error: spawn can't get null entry point\n");
        return -1;
    }
    char *stack = static_cast<char *>(malloc(STACK_SIZE));
    mask_signals();
    int curId = scheduler->addThread(stack, entry_point);
    unmask_signals();
    return curId;
}

int uthread_sleep(int num_quantums){
    mask_signals();
    if(num_quantums <= 0){
        fprintf(stderr, "thread library error: the num_quantums need to be non negative\n");
        return -1;
    }
    if (curRunning->id == MAIN_THREAD_TID){
        fprintf(stderr, "thread library error: main thread can't sleep\n");
        return -1;
    }
    curRunning->isSleep = true;
    curRunning->waitingTime = num_quantums;
    curRunning->status = READY;
    dispatcher->launchTimer();
    Dispatcher::switchThread(MANUAL_SWITCH);
    return 0;
}
