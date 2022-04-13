
#ifndef OS_EX2_POOLMANAGER_H
#define OS_EX2_POOLMANAGER_H

#include "uthreads.h"
#include "Thread.h"
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <csignal>
#include <assert.h>


#define READY 1
#define RUNNING 2
#define BLOCKED 3
//#define TERMINATED 4
#define MAIN_THREAD_TID 0

/* External interface */
#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

using namespace std;

class PoolManager {

  //fields:
 private:
//TODO return fields to private

 public:
    inline static Thread* curRunning;
 set<Thread *> *blockedID;
 // std::map<int, Thread *> *allThreads;
 std::deque<Thread *> *readyQueue;
 Thread* allThreads[MAX_THREAD_NUM] = {0};
 int counter;
 int countReady;

  //Methods
  PoolManager ()
  {
      countReady = 1; //should start from 1?
//    counter = 1;
    curRunning = nullptr;
    blockedID = new set<Thread *> ();
//    allThreads = new map<int, Thread *> ();
      readyQueue = new deque< Thread *> ();
  }

  void initMainThread ()
  {
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
      curRunning = mainThread;
      curRunning = mainThread;
      sigsetjmp(mainThread->env, 1);
  }

  int findId(){
      for(int i = 1; i < MAX_THREAD_NUM; i++){
          if(allThreads[i] == nullptr){
              return i;
          }
      }
  }
  
  int addThread (char *stack, thread_entry_point entry_point)
  {
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
    counter++;
    countReady++;
    allThreads[newTread->id]  = newTread;
    readyQueue->push_back(newTread);
    newTread->duplicate = 1;
    return newTread->id;
  }

//  void finalTerminate (Thread *thread)
//  {
//    if (thread->duplicate <= 1 || thread->status == TERMINATED)
//      {
//
//      }
//    else
//      {
//        thread->duplicate--;
//      }
//  }

    Thread *nextAvailableReady ()
  {
      if(readyQueue->empty()){
          return allThreads[0];
      }
    Thread *candidate = readyQueue->front();
    readyQueue->pop_front();
    candidate->duplicate--;
    while ((candidate->status != READY) || (candidate->duplicate == 1))
      {
//        if (candidate->status == TERMINATED)
//          {
//            finalTerminate (candidate);
//          }
        if(readyQueue->empty()){
            return curRunning;
        }
        candidate = readyQueue->front ();
        readyQueue->pop_front();
        candidate->duplicate--;
      }
    return candidate;
  }
//
//  Thread *getThreadById (int id)
//  {
//    auto res = (*allThreads).find (id);
//    if (res != (*allThreads).end ())
//      {
//        return res->second;
//      }
//    else
//      {
//        return nullptr;
//      }
//  }

  int blockThread (int tid)
  {
     Thread* thread = allThreads[tid];
     thread->status = BLOCKED;
     auto ret = blockedID->insert(thread);
     return ret.second;
  }

  void preemptedThread ()
  {
    curRunning->status = READY;
    readyQueue->push_back(curRunning);
    curRunning->duplicate++;

  }

  int resumeThread (int tid)
  {
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
      //CHECK IF IT IN THE READY LIST
//      auto it = find(readyQueue->begin(), readyQueue->end(), thread);
//      if(it == readyQueue->end())
//      {
//          readyQueue->push_back(thread);
//          return 0;
//      }
      return -1;
  }

  void terminateProcess ()
  {
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

  int terminateThread (int tid)
  {
      Thread* thread = allThreads[tid];
      if(thread == nullptr){
      return -1;
    }
      readyQueue->erase(remove(readyQueue->begin(), readyQueue->end(), thread), readyQueue->end());
    allThreads[tid] = nullptr;
    delete thread->stackPointer;
    delete thread;
    countReady--;

    return 0;
  }

  void setRunning (Thread * thread){
    thread->status = RUNNING;
    PoolManager::curRunning = thread;
  }


  unsigned long count(){
//      return allThreads->size();
    return countReady;
  }

  void updateWaitingTime(){
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
              }
          }
      }
  }
};
#endif



