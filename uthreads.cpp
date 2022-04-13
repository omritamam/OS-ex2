#include <cstdlib>
#include <cstdio>
#include <armadillo>
#include "PoolManager.h"
#include "Starter.h"

inline static PoolManager *pool;
inline static Starter *starter;
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
// yes - pool.moveToDelete + starter.start
// no - pool.moveToDelete
int uthread_terminate (int tid)
{
    Starter::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    { //main thread terminated
        pool->terminateProcess ();
        delete starter;
        delete pool;
        exit(0);
    }
    if (PoolManager::curRunning->id == tid)
    { //terminate himself
        if (pool->terminateThread (tid))
        {
            return -1;
        }
        starter->launchTimer();
        starter->switchThread (0);
    }
    if (pool->terminateThread (tid))
    {
        return -1;
    }
    Starter::unmask_signals();
    return 0;
}

int uthread_block(int tid)
{
    Starter::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    {
        fprintf(stderr, "thread library error: it's illegal to block the main thread\n");
        return -1;
    }
    pool->blockThread(tid);
    if (PoolManager::curRunning->id == tid)
    {
        //block himself
        starter->launchTimer();
        starter->switchThread (MANUAL_SWITCH);
    }
    Starter::unmask_signals();
    return 0;
}


int uthread_resume(int tid)
{
    Starter::mask_signals();
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if(!pool->allThreads[tid]->isSleep){
        pool->resumeThread(tid);
    }
    pool->allThreads[tid]->status = READY;
    Starter::unmask_signals();
    return 0;
};


int uthread_get_tid(){
    return pool->curRunning->id;
}

int uthread_get_total_quantums(){
    return starter->totalQuantum;
}

int uthread_get_quantums(int tid){
    if(checkValidTid(tid) == -1){
        return -1;
    }
    return pool->allThreads[tid]->quantum;
}

int uthread_init (int quantum_usecs)
{
    starter = new Starter();
    pool = new PoolManager();
    starter->init(pool);

    if (quantum_usecs <= 0)
    {
        fprintf (stderr, "thread library error: quantum_usecs must be positive\n");
        return -1;
    }
    starter->initTimer(quantum_usecs);
    pool->initMainThread();
    Starter::unmask_signals();
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
    Starter::mask_signals();
    int curId = pool->addThread(stack, entry_point);
    Starter::unmask_signals();
    return curId;
}

int uthread_sleep(int num_quantums){
    Starter::mask_signals();
    if(num_quantums <= 0){
        fprintf(stderr, "thread library error: the num_quantums need to be non negative\n");
        return -1;
    }
    if (PoolManager::curRunning->id == MAIN_THREAD_TID){
        fprintf(stderr, "thread library error: main thread can't sleep\n");
        return -1;
    }
    PoolManager::curRunning->isSleep = true;
    PoolManager::curRunning->waitingTime = num_quantums;
    PoolManager::curRunning->status = READY;
    starter->launchTimer();
    Starter::switchThread (MANUAL_SWITCH);
    return 0;
}
