//
// Created by omri.tamam on 04/04/2022.
//

#include <csignal>
#include "Utils.h"
class Utils{
    static void mask_signals(bool mask){
        if(mask){
            if(sigprocmask(SIG_BLOCK, &sig_set, nullptr) == -1){
                //print sigerr
            }
        }
        else{
            if(sigprocmask(SIG_UNBLOCK, &sig_set, nullptr) == -1){
                //print sigerr
            }
        }
    }
};
