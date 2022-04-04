#include "Starter.h"

int Starter::init(PoolManager *staticPool) {
    pool = staticPool;
    totalQuantum = 1;
}

void Starter::switchThread(int sig) {
    int ret_val = sigsetjmp(Starter::pool->curRunning->env, 1);
    if (ret_val == RETURN_FROM_STARTER)
    {
        //unblock_signals ();
        return;
    }
    printf ("stop the thread!!!");
    fflush (stdout);
    Starter::pool->preemptedThread();
    //should check if there is no next available?
    Thread *nextThread = Starter::pool->nextAvailableReady ();
    Starter::pool->setRunning(nextThread->id);
    Starter::totalQuantum++;
    PoolManager::curRunning->quantum++;
    //TODO error
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);
}



