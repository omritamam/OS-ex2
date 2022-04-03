#include "Timer.h"

static PoolManager* pool;
static Starter* starter;
static Timer* timer;


int uthread_init2(int quantum_usecs){
    pool =  new PoolManager();
    starter = new Starter();
    timer = new Timer(quantum_usecs);


    if (quantum_usecs < 0){
        printf("input error invalid quantum usedcs");
        return -1;
    }
    timer->Init();




}

// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn2(thread_entry_point entry_point){

}

// check if it terminates itself -
// yes - pool.moveToDelete + starter.start
// no - pool.moveToDelete
int uthread_terminate2(int tid){

}



int uthread_block2(int tid){

}


// if already running - fails, else - pool.move(block, ready)
int uthread_resume2(int tid){

};

int main(void)
{
    uthread_init2(2000000);
    for(;;){

    }
}


