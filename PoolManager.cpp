#include "PoolManager.h"

void PoolManager::setRunning (int tid)
{
    Thread *curThread = getThreadById (tid);
    curThread->status = RUNNING;
    PoolManager::curRunning = curThread;
}