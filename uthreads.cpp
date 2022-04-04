#include <cstdlib>
#include <cstdio>
#include <armadillo>
#include "PoolManager.h"
#include "Starter.h"

class Uthreads{
    inline static PoolManager *pool;
    inline static Starter *starter;

    int SECOND = 6000000;

    void initMainThread ();

    // check if it terminates itself -
    // yes - pool.moveToDelete + starter.start
    // no - pool.moveToDelete
    int uthread_terminate (int tid)
    {
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
            starter->switchThread (0);
        }
    }

    int uthread_block(int tid)
    {
        if (tid == MAIN_THREAD_TID)
        {
            return 1;
        }
        pool->blockThread(tid);
        if (PoolManager::curRunning->id == tid)
        { //terminate himself
            starter->switchThread (0);
        }
    }

    // if already running - fails, else - pool.move(block, ready)
    int uthread_resume(int tid)
    {
        if(pool->resumeThread(tid)){
            //TODO- error
            return -1;
        }
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

    static int uthread_get_total_quantums(){
        return starter->totalQuantum;
    }

    int uthread_get_quantums(int tid){
        return pool->getThreadById(tid)->quantum;
    }

public:
    static int uthread_init (int quantum_usecs)
    {
        starter = new Starter();
        starter->init(pool);
        starter->initTimer(quantum_usecs);
        pool = new PoolManager();

        if (quantum_usecs < 0)
        {
            printf ("input error invalid quantum usedcs");
            return -1;
        }
        pool->initMainThread();
        //    timer->Init();
    }

// equals to setup threads. tid comes from Pool, push to pool.ready
static int uthread_spawn (thread_entry_point entry_point)
{
    if(pool->count()>=MAX_THREAD_NUM){
        return 1;
    }
    char *stack = static_cast<char *>(malloc(STACK_SIZE));
    pool->addThread (stack, entry_point);
}
};



//void thread0 (void)
//{
//  int i = 0;
//  while (1)
//    {
//      ++i;
//      printf ("in thread0 (%d)\n");
//      usleep (100000);
//    }
//}
//
//void thread1 (void)
//{
//  int i = 0;
//  while (1)
//    {
//      printf ("in thread1 (%d)\n", i);
//      usleep (15000000);
//      uthread_terminate(uthread_get_tid());
//      i = i+2;
//    }
//}

int main (void)
{
//  Uthreads::uthread_init (2000000);
////  uthread_spawn (thread0);
//  Uthreads::uthread_spawn (thread1);

}


