unsigned int MAX_THREAD_NUM = 3;
unsigned int STACK_SIZE = 4096;
PoolManager pool;
Starter starter;

static PoolManager* pool;
static Starter* starter;

int uthread_init2(int quantum_usecs){
    pool =  new PoolManager();
    starter = new Starter();

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


