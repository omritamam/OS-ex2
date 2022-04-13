
#include "Scheduler.h"

int Scheduler::addThread(char *stack, thread_entry_point entry_point) {
    Thread *newTread = new Thread ();
    newTread->stackPointer = stack;
    newTread->id = findId();
    newTread->status = READY;
    newTread->waitingTime = -1;
    newTread->isSleep = false;
    address_t sp = (address_t) stack + STACK_SIZE - sizeof (address_t);
    address_t pc = (address_t) entry_point;
    sigsetjmp(newTread->env, 1);
    ((newTread->env)->__jmpbuf)[JB_SP] = translate_address (sp);
    ((newTread->env)->__jmpbuf)[JB_PC] = translate_address (pc);
    sigemptyset (&(newTread->env)->__saved_mask);
    countReady++;
    allThreads[newTread->id]  = newTread;
    readyQueue->push_back(newTread);
    newTread->duplicate = 1;
    return newTread->id;
}

Scheduler::Scheduler() {
    countReady = 1;
    curRunning = nullptr;
    blockedID = new set<Thread *> ();
    readyQueue = new deque< Thread *> ();
}

void Scheduler::initMainThread() {
    Thread *mainThread = new Thread ();
    mainThread->stackPointer = new char(1);
    mainThread->id = MAIN_THREAD_TID;
    mainThread->status = RUNNING;
    mainThread->duplicate = 0;
    mainThread->quantum = 1;
    mainThread->waitingTime = -1;
    mainThread->isSleep = false;
    sigemptyset (&(mainThread->env)->__saved_mask);
    allThreads[mainThread->id] =  mainThread;
    Scheduler::curRunning = mainThread;
    sigsetjmp(mainThread->env, 1);
}

int Scheduler::findId() {
    for(int i = 1; i < MAX_THREAD_NUM; i++){
        if(allThreads[i] == nullptr){
            return i;
        }
    }
    return -1;
}

Thread *Scheduler::nextAvailableReady() {
    if(readyQueue->empty()){
        return allThreads[0];
    }
    Thread *candidate = readyQueue->front();
    readyQueue->pop_front();
    candidate->duplicate--;
    while ((candidate->status != READY) || (candidate->duplicate == 1))
    {

        if(readyQueue->empty()){
            return curRunning;
        }
        candidate = readyQueue->front ();
        readyQueue->pop_front();
        candidate->duplicate--;
    }
    return candidate;
}

int Scheduler::blockThread(int tid) {
    Thread* thread = allThreads[tid];
    thread->status = BLOCKED;
    auto ret = blockedID->insert(thread);
    return ret.second;
}

void Scheduler::preemptedThread() {
    curRunning->status = READY;
    readyQueue->push_back(curRunning);
    curRunning->duplicate++;

}

void Scheduler::terminateProcess() {
    for (auto & thread : allThreads)
    {
        if(thread != nullptr)
            delete thread->stackPointer;
        delete thread;
    }
    allThreads[MAX_THREAD_NUM] = {0};

    delete blockedID;
    delete readyQueue;
}

int Scheduler::terminateThread(int tid) {
    Thread* thread = allThreads[tid];
    if (thread == nullptr){
        return -1;
    }
//TODO: FIX!!
//      auto pos = find((readyQueue->begin()), (readyQueue->end()), &thread);
//      readyQueue->erase(pos);
    allThreads[tid] = nullptr;
    delete thread->stackPointer;
    delete thread;
    countReady--;
    return 0;
}

void Scheduler::updateWaitingTime() {
    for (auto & thread : allThreads)
    {
        if(thread != nullptr){
            if(thread->isSleep){
                thread->waitingTime--;
            }
            if(thread->waitingTime == 0){
                thread->isSleep = false;
                if(thread->status == READY){
                    readyQueue->push_back(thread);
                    thread->duplicate++;
                }
                thread->waitingTime = -1;
            }
        }
    }
}

unsigned long Scheduler::count() {
    return countReady;
}

void Scheduler::setRunning(Thread *thread) {
    thread->status = RUNNING;
    curRunning = thread;
}

int Scheduler::resumeThread(int tid) {
    Thread* thread = allThreads[tid];
    if (thread->status == BLOCKED)
    {
        blockedID->erase(thread);
        thread->status = READY;
        if(!thread->isSleep){
            readyQueue->push_back(thread);
            thread->duplicate++;
        }
        return 0;
    }
    return -1;
}

