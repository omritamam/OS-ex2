unsigned int MAX_THREAD_NUM = 3;
unsigned int STACK_SIZE = 4096;
PoolManager pool;
Starter starter;

int uthread_init(int quantum_usecs){
    pool =  PoolManagar();

}

// equals to setup threads. tid comes from Pool, push to pool.ready
int uthread_spawn(thread_entry_point entry_point){

}

int uthread_terminate(int tid){

}

int uthread_block(int tid){

}


// if already running - fails, else - pool.move(block, ready)
int uthread_resume(int tid);



