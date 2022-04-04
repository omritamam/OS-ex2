#include <armadillo>
#include "Starter.h"
int RETURN_FROM_STARTER = 5;

int Starter::init(PoolManager *staticPool) {
    pool = staticPool;
    totalQuantum = 1;
}

void Starter::switchThread(int sig) {
    mask_signals(true);
    write(1,"end thread\n",11);
    int ret_val = setjmp(Starter::pool->curRunning->env);
    if (ret_val == RETURN_FROM_STARTER)
    {
        write(1,"start thread\n",13);
        mask_signals(false);
        return;
    }
    if(PoolManager::curRunning->status == TERMINATED){
        pool->finalTerminate(PoolManager::curRunning);
    }
    Starter::pool->preemptedThread();
    //should check if there is no next available?
    Thread *nextThread = Starter::pool->nextAvailableReady ();
    Starter::pool->setRunning(nextThread->id);
    Starter::totalQuantum++;
    PoolManager::curRunning->quantum++;
    //TODO error
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);
}



