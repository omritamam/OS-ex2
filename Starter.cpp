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
//log("end thread ");
    int ret_val = setjmp(Starter::pool->curRunning->env);
    if (ret_val == RETURN_FROM_STARTER)
    {
        //log("start thread ");
        unmask_signals();
        return;
    }
#pragma region handle old thread
    if(PoolManager::curRunning->status == TERMINATED){
        pool->finalTerminate(PoolManager::curRunning);
    }
    if(PoolManager::curRunning->status == RUNNING){
        Starter::pool->preemptedThread();
    }
#pragma endregion
    //should check if there is no next available?
    Thread *nextThread = Starter::pool->nextAvailableReady();
    Starter::pool->setRunning(nextThread);
    Starter::totalQuantum++;
    PoolManager::curRunning->quantum++;
    if(PoolManager::curRunning->quantum > 7 && PoolManager::curRunning->id == 0){
        fprintf(stderr, "gotya!");
    }
    //TODO error
    unmask_signals();
    siglongjmp(nextThread->env, RETURN_FROM_STARTER);

}

void Starter::log(const char *prefix) {
    char* message;
    int index = pool->curRunning->id;
    string str_obj(prefix);
    str_obj = str_obj + to_string(index);
    str_obj = str_obj + '\n';
    message = &str_obj[0];
    write(1,message,12);
}



