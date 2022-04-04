/*
 * sigsetjmp/siglongjmp demo program.
 * Hebrew University OS course.
 * Author: OS, os@cs.huji.ac.il
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
extern address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
        "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
extern address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}


#endif

#define STACK_SIZE 4096


struct itimerval timer;

typedef void (*thread_entry_point)(void);

char stack0[STACK_SIZE];
char stack1[STACK_SIZE];
sigjmp_buf env[2];
int current_thread = -1;


void jump_to_thread(int tid)
{
    current_thread = tid;
    siglongjmp(env[tid], 5);
}

/**
 * @brief Saves the current thread state, and jumps to the other thread.
 */
void yield(void)
{
    int ret_val = sigsetjmp(env[current_thread], 1);
    printf("yield: ret_val=%d\n", ret_val);
    bool did_just_save_bookmark = ret_val == 0;
//    bool did_jump_from_another_thread = ret_val != 0;
    if (did_just_save_bookmark)
    {
        jump_to_thread(1 - current_thread);
    }
}
void switch_handler(int sig)
{
    printf("inside switch_handler");
    int ret_val = sigsetjmp(env[current_thread], 1);
//     (env[tid]->__jmpbuf)[JB_PC] = translate_address(?????);

    bool did_just_save_bookmark = ret_val == 0;
    //    bool did_jump_from_another_thread = ret_val != 0;
    if (did_just_save_bookmark)
    {
        jump_to_thread(1 - current_thread);
    }

}



void thread0(void)
{
    int i = 0;
    while (1)
    {
        ++i;
        printf("in thread0 (%d)\n", i);
//        struct itimerval new_timer;
//        getitimer(ITIMER_VIRTUAL,&new_timer);
//        printf("time in timer %ld\n", new_timer.it_value.tv_sec);
        usleep(USEC_IN_SEC);
    }

}


void thread1(void)
{
    int i = 0;
    while (1)
    {
        ++i;
        printf("in thread1 (%d)\n", i);
        usleep(USEC_IN_SEC);
    }
}


void setup_thread(int tid, char *stack, thread_entry_point entry_point)
{
    // initializes env[tid] to use the right stack, and to run from the function 'entry_point', when we'll use
    // siglongjmp to jump into the thread.
    address_t sp = (address_t) stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t) entry_point;
    sigsetjmp(env[tid], 1);
    (env[tid]->__jmpbuf)[JB_SP] = translate_address(sp);
    (env[tid]->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&env[tid]->__saved_mask);
}


void setup(void)
{
    // sets up the threads, so they'll have the wanted initial SP and PC.
    setup_thread(0, stack0, thread0);
    setup_thread(1, stack1, thread1);
}


int main(void)
{
    setup();
    struct sigaction sa = {0};

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &switch_handler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error.");
    }

    // configure the timer to expire every 5 sec after that.
    timer.it_interval.tv_sec = 3;    // following time intervals, seconds part
    timer.it_interval.tv_usec = 0;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        printf("setitimer error.");
    }


    jump_to_thread(0);
    return 0;
}
