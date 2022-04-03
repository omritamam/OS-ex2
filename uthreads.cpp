#include <cstdlib>
#include "Timer.h"

static PoolManager *pool;
static Starter *starter;
static Timer *timer;

void initMainThread ();
// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn2 (thread_entry_point entry_point)
{
  char *stack = new char (STACK_SIZE);
  pool->addThread (stack, entry_point);
}

int uthread_init2 (int quantum_usecs)
{
  pool = new PoolManager ();
  starter = new Starter (pool);
  timer = new Timer (quantum_usecs);

  if (quantum_usecs < 0)
    {
      printf ("input error invalid quantum usedcs");
      return -1;
    }
    pool->initMainThread();
//    timer->Init();
}

// check if it terminates itself -
// yes - pool.moveToDelete + starter.start
// no - pool.moveToDelete
int uthread_terminate2 (int tid)
{
  if (tid == MAIN_THREAD_TID)
    { //main thread terminated
      pool->terminateProcess ();
      delete starter;
      delete pool;
      delete timer;
      exit(0);
    }
  if (pool->terminateThread (tid))
    {
      return -1;
    }
  if (pool->curRunning->id == tid)
    { //terminate himself
      starter->switchThread (0);
    }
}

int uthread_block2 (int tid)
{
  if (tid == MAIN_THREAD_TID)
    {
      return 1;
    }
  pool->blockThread(tid);
  if (pool->curRunning->id == tid)
    { //terminate himself
      starter->switchThread (0);
    }
}

// if already running - fails, else - pool.move(block, ready)
int uthread_resume2 (int tid)
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
int uthread_get_tid2(){
  return pool->curRunning->id;
}

int uthread_get_total_quantums2(){
  return starter->totalQuantum;
}

int uthread_get_quantums2(int tid){
  return pool->getThreadById(tid)->quantum;
}


void thread0 (void)
{
  int i = 0;
  while (1)
    {
      ++i;
      printf ("in thread0 (%d)\n", i);
      usleep (SECOND);
    }
}

void thread1 (void)
{
  int i = 0;
  while (1)
    {
      ++i;
      printf ("in thread1 (%d)\n", i);
      usleep (SECOND);
      uthread_terminate2(uthread_get_tid2());
    }
}

int main (void)
{
  uthread_init2 (2000000);
//  uthread_spawn2 (thread0);
  uthread_spawn2 (thread1);

}


