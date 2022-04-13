#include <cstdlib>
#include <cstdio>
#include <armadillo>
#include "Scheduler.h"
#include "Dispather.h"
 static Scheduler *pool;
 static Dispather *dispather;
#define MANUAL_SWITCH 4
using namespace std;

int checkValidTid(int tid){
    if ( tid < 0 || tid > 99 )
    {
        fprintf(stderr, "thread library error: the thread id is invalid (it needs to be  between 0 to 99)\n");
        return -1;
    }
    if ( pool->allThreads[tid] == nullptr)
    {
        fprintf(stderr, "thread library error: the thread id's cell is empty in the threadsArr\n");
        return -1;
    }
    return 0;
}

// check if it terminates itself -
// yes - pool.moveToDelete + dispather.start
// no - pool.moveToDelete
int uthread_terminate (int tid)
{
    Dispather::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    { //main thread terminated
        pool->terminateProcess ();
        delete dispather;
        delete pool;
        exit(0);
    }
    if (Scheduler::curRunning->id == tid)
    { //terminate himself
        if (pool->terminateThread (tid))
        {
            return -1;
        }
        dispather->launchTimer();
        dispather->switchThread (0);
    }
    if (pool->terminateThread (tid))
    {
        return -1;
    }
    Dispather::unmask_signals();
    return 0;
}

int uthread_block(int tid)
{
    Dispather::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    {
        fprintf(stderr, "thread library error: it's illegal to block the main thread\n");
        return -1;
    }
    pool->blockThread(tid);
    if (Scheduler::curRunning->id == tid)
    {
        //block himself
        dispather->launchTimer();
        dispather->switchThread (MANUAL_SWITCH);
    }
    Dispather::unmask_signals();
    return 0;
}


int uthread_resume(int tid)
{
    Dispather::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if(!pool->allThreads[tid]->isSleep){
        pool->resumeThread(tid);
    }
    pool->allThreads[tid]->status = READY;
    Dispather::unmask_signals();
    return 0;
};


int uthread_get_tid(){
    return pool->curRunning->id;
}

int uthread_get_total_quantums(){
    return dispather->totalQuantum;
}

int uthread_get_quantums(int tid){
    if(checkValidTid(tid) == -1){
        return -1;
    }
    return pool->allThreads[tid]->quantum;
}

int uthread_init (int quantum_usecs)
{
    dispather = new Dispather();
    pool = new Scheduler();
    dispather->init(pool);

    if (quantum_usecs <= 0)
    {
        fprintf (stderr, "thread library error: quantum_usecs must be positive\n");
        return -1;
    }
    dispather->initTimer(quantum_usecs);
    pool->initMainThread();
    Dispather::unmask_signals();
    return 0;
}

// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn (thread_entry_point entry_point)
{
    if(pool->count()>=MAX_THREAD_NUM){
        fprintf(stderr, "thread library error: reached the max number\n");
        return -1;
    }
    if(entry_point == nullptr){
        fprintf(stderr, "thread library error: spawn can't get null entry point\n");
        return -1;
    }
    char *stack = static_cast<char *>(malloc(STACK_SIZE));
    Dispather::mask_signals();
    int curId = pool->addThread(stack, entry_point);
    Dispather::unmask_signals();
    return curId;
}

int uthread_sleep(int num_quantums){
    Dispather::mask_signals();
    if(num_quantums <= 0){
        fprintf(stderr, "thread library error: the num_quantums need to be non negative\n");
        return -1;
    }
    if (Scheduler::curRunning->id == MAIN_THREAD_TID){
        fprintf(stderr, "thread library error: main thread can't sleep\n");
        return -1;
    }
    Scheduler::curRunning->isSleep = true;
    Scheduler::curRunning->waitingTime = num_quantums;
    Scheduler::curRunning->status = READY;
    dispather->launchTimer();
    Dispather::switchThread (MANUAL_SWITCH);
    return 0;
}
int main(){
    return 0;
}
