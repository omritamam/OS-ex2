#ifndef OS_EX2_TERMINATOR_H
#define OS_EX2_TERMINATOR_H

#include "PoolManager.h"
#include "Starter.h"


class Terminator{

    public:
    static void terminate(int sig){
        printf("terminate!");
        fflush(stdout);
        int ret_val = sigsetjmp(PoolManager::curRunning->env,1);
        if (ret_val == RETURN_FROM_STARTER) {
          //TODO - maybe set running here?
            unblock_signals();
            return;
        }
    }


};


#endif
