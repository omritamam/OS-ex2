
#ifndef OS_EX2_STARTER_H
#define OS_EX2_STARTER_H
int RETURN_FROM_STARTER = 5;

class Starter {

 private:
  static PoolManager *pool;

 public:
  static int totalQuantum;
  Starter (PoolManager *staticPool)
  {
    pool = staticPool;
    totalQuantum = 1;
  };

  static void switchThread (int sig)
  {
    int ret_val = sigsetjmp(pool->curRunning->env, 1);
    if (ret_val == RETURN_FROM_STARTER)
      {
        unblock_signals ();
        return;
      }
    printf ("stop the thread!!!");
    fflush (stdout);
    pool->preemptedThread ();
    //should check if there is no next available?
    Thread *nextThread = pool->nextAvailableReady ();
    pool->setRunning (nextThread->id);
    totalQuantum++;
    pool->curRunning->quantum++;
    //TODO error
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);
  }


};
#endif