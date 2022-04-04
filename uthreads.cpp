#include <cstdlib>
#include <cstdio>
#include <armadillo>
#include "PoolManager.h"
#include "Starter.h"

inline static PoolManager *pool;
inline static Starter *starter;

int SECOND = 6000000;


// check if it terminates itself -
// yes - pool.moveToDelete + starter.start
// no - pool.moveToDelete
int uthread_terminate (int tid)
{
    Starter::mask_signals(true);
    write(1,"terminated\n",11);
    if (tid == MAIN_THREAD_TID)
    { //main thread terminated
        pool->terminateProcess ();
        delete starter;
        delete pool;
        exit(0);
    }
    if (pool->terminateThread (tid))
    {
        return -1;
    }
    if (PoolManager::curRunning->id == tid)
    { //terminate himself
        PoolManager::curRunning->status= TERMINATED;
        starter->switchThread (0);
    }
}

int uthread_block(int tid)
{
    Starter::mask_signals(true);

    if (tid == MAIN_THREAD_TID)
    {
        return 1;
    }
    pool->blockThread(tid);
    if (PoolManager::curRunning->id == tid)
    { //terminate himself
        pool->terminateThread(tid);
        starter->switchThread (0);
    }
}

// if already running - fails, else - pool.move(block, ready)
int uthread_resume(int tid)
{
    Starter::mask_signals(true);

    if(pool->resumeThread(tid)){
        //TODO- error
        return -1;
    }
    Starter::mask_signals(false);

    return 0;
};

/**
 * @brief Returns the thread ID of the calling thread.
 *
 * @return The ID of the calling thread.
*/
int uthread_get_tid(){
    return pool->curRunning->id;
}

int uthread_get_total_quantums(){
    return starter->totalQuantum;
}

int uthread_get_quantums(int tid){
    return pool->getThreadById(tid)->quantum;
}

int uthread_init (int quantum_usecs)
{
    starter = new Starter();
    pool = new PoolManager();
    starter->init(pool);

    if (quantum_usecs < 0)
    {
        printf ("input error invalid quantum usedcs");
        return -1;
    }
    pool->initMainThread();
    starter->initTimer(quantum_usecs);
}

// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn (thread_entry_point entry_point)
{
    if(pool->count()>=MAX_THREAD_NUM){
        return 1;
    }
    char *stack = static_cast<char *>(malloc(STACK_SIZE));
    pool->addThread (stack, entry_point);
}

int uthread_sleep(int num_quantums){
    return 0;
}
