#include <cstdlib>
#include <cstdio>
#include <armadillo>
#include "PoolManager.h"
#include "Starter.h"

inline static PoolManager *pool;
inline static Starter *starter;
using namespace std;

int checkValidTid(int tid){
    if ( tid < 0 || tid > 99 )
    {
        fprintf(stderr, "thread library error: the thread id is invalid (it needs to be  between 0 to 99)\n");
        return -1;
    }
    if ( pool->isUsed[tid]==0 )
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
//    cout << "count before" <<pool->count()<<endl;
    Starter::mask_signals(true);
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
    if (pool->terminateThread (tid))
    {

        return -1;
    }
    if (PoolManager::curRunning->id == tid)
    { //terminate himself
        PoolManager::curRunning->status= TERMINATED;
        starter->initTimer();
        starter->switchThread (0);
    }
}

int uthread_block(int tid)
{
    Starter::mask_signals(true);
    if(checkValidTid(tid) == -1){
        return -1;
    }
    if (tid == MAIN_THREAD_TID)
    {
        fprintf(stderr, "thread library error: it's illegal to block the main thread\n");
        return -1;
    }
    pool->blockThread(tid);
//    cout <<"try3" << endl;
    if (PoolManager::curRunning->id == tid)
    { //terminate himself
        starter->initTimer();
        starter->switchThread (0);
    }
    Starter::mask_signals(false);


    //print state
//    set<Thread*>::iterator itr;
//    cout << "blocked IDs:"<<endl;
//    for (itr = pool->blockedID->begin();
//    itr != pool->blockedID->end(); itr++)
//    {
//        cout << *itr << " ";
//    }
}


// if already running - fails, else - pool.move(block, ready)
int uthread_resume(int tid)
{
    Starter::mask_signals(true);
    if(checkValidTid(tid) == -1){
        return -1;
    }
    pool->resumeThread(tid);
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
    if(checkValidTid(tid) == -1){
        return -1;
    }
    return pool->getThreadById(tid)->quantum;
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
    Starter::mask_signals(false);
}

// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn (thread_entry_point entry_point)
{
    if(pool->count()>=MAX_THREAD_NUM){
        cout << "reached the max number" << endl; //TODO - remove to stderr
        return -1;
    }
    char *stack = static_cast<char *>(malloc(STACK_SIZE));
    Starter::mask_signals(true);
    int curId = pool->addThread(stack, entry_point);
    Starter::mask_signals(false);
    return curId;
}

int uthread_sleep(int num_quantums){
    return 0;
}
