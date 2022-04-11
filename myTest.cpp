//
// Created by shirayarhi on 10/04/2022.
//

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include "uthreads.h"
//#include "libuthread.a"
#include <iostream>

void thread0(void)
{
    int i = 0;
    printf("reached thread0 (%d)\n", i);
    while (uthread_get_quantums(uthread_get_tid())<2)
    {
    }
    uthread_terminate(0);
}


void thread1(void)
{
    int i = 0;
    printf("reached thread1 (%d)\n", i);
    while (uthread_get_quantums(uthread_get_tid())<2)
    {
    }
    uthread_terminate(1);
}

int main(){
    uthread_init(2*10);
    uthread_spawn(thread0);
    uthread_spawn(thread1);
    while(uthread_get_total_quantums()<6){}
    std::cout<<"total quantums: "<<uthread_get_total_quantums()<<std::endl;
}