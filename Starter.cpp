#include <armadillo>
#include "Starter.h"
int RETURN_FROM_STARTER = 5;

int Starter::init(PoolManager *staticPool) {
    pool = staticPool;
    totalQuantum = 1;
    return 0;
}

void Starter::switchThread(int sig) {
    mask_signals();
    int ret_val = setjmp(Starter::pool->curRunning->env);
    if (ret_val == RETURN_FROM_STARTER)
    {
        unmask_signals();
        return;
    }
#pragma region handle old thread
    if(PoolManager::curRunning->status == RUNNING && (!PoolManager::curRunning->isSleep)){
        Starter::pool->preemptedThread();
    }
#pragma endregion
    //should check if there is no next available?
    Thread *nextThread = Starter::pool->nextAvailableReady();
    Starter::pool->setRunning(nextThread);
    Starter::totalQuantum++;
    pool->updateWaitingTime();
    PoolManager::curRunning->quantum++;
    //TODO error
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);

}




