
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
#define TERMINATED 4
#define MAIN_THREAD_TID 0

/* External interface */
#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

using namespace std;

class PoolManager {

  //fields:
 private:
    set<Thread *> *blockedID;
  std::map<int, Thread *> *allThreads;
  std::queue<Thread *> *IDQueue;
  int counter;


 public:
    inline static Thread* curRunning;


  //Methods
  PoolManager ()
  {
    counter = 1;
    curRunning = nullptr;
    blockedID = new set<Thread *> ();
    allThreads = new map<int, Thread *> ();
    IDQueue = new queue< Thread *> ();
  }

  void initMainThread ()
  {
      Thread *mainThread = new Thread ();
      mainThread->stackPointer = new char(1);
      mainThread->id = MAIN_THREAD_TID;
      mainThread->status = READY;
      mainThread->duplicate = 0;
      sigemptyset (&(mainThread->env)->__saved_mask);
      allThreads->insert (pair<int, Thread *> (mainThread->id, mainThread));
      curRunning = mainThread;
      sigsetjmp(mainThread->env, 1);

  }

  void addThread (char *stack, thread_entry_point entry_point)
  {
    Thread *newTread = new Thread ();
    newTread->stackPointer = stack;
    newTread->id = counter;
    newTread->status = READY;
    address_t sp = (address_t) stack + STACK_SIZE - sizeof (address_t);
    address_t pc = (address_t) entry_point;
    sigsetjmp(newTread->env, 1);
    ((newTread->env)->__jmpbuf)[JB_SP] = translate_address (sp);
    ((newTread->env)->__jmpbuf)[JB_PC] = translate_address (pc);
    sigemptyset (&(newTread->env)->__saved_mask);
    counter++;
    allThreads->insert (pair<int, Thread *> (newTread->id, newTread));
    IDQueue->push(newTread);
    newTread->duplicate=1;
  }

  void finalTerminate (Thread *thread)
  {
    if (thread->duplicate <= 1 || thread->status == TERMINATED)
      {
        allThreads->erase (thread->id);
        delete thread->stackPointer;
        delete thread;
      }
    else
      {
        thread->duplicate--;
      }
  }

  Thread *nextAvailableReady ()
  {
    Thread *candidate = IDQueue->front();
    IDQueue->pop();
      assert(NULL != candidate);
    candidate->duplicate--;
    while ((candidate->status != READY) || (candidate->duplicate > 0))
      {
        if (candidate->status == TERMINATED)
          {
            finalTerminate (candidate);
          }
        candidate = IDQueue->front ();
        IDQueue->pop();
      }
    return candidate;
  }

  Thread *getThreadById (int id)
  {
    auto res = (*allThreads).find (id);
    if (res != (*allThreads).end ())
      {
        return res->second;
      }
    else
      {
        return nullptr;
      }
  }

  int blockThread (int tid)
  {
     Thread* thread = getThreadById (tid);
     thread->status = BLOCKED;
     auto ret = blockedID->insert (thread);
    return ret.second;
  }

  void preemptedThread ()
  {
    curRunning->status = READY;
    IDQueue->push(curRunning);
    curRunning->duplicate++;
  }

  int resumeThread (int tid)
  {
      Thread* thread = getThreadById(tid);
      auto res = blockedID->find (thread);
      if (res != blockedID->end())
      {
          blockedID->erase (thread);
        IDQueue->push (thread);
          thread->status = READY;
        thread->duplicate++;
        return 0;
      }
      //Thread not exist
      return 1;

  }

  void terminateProcess ()
  {
    for (const auto &kv : *allThreads)
      {
        delete kv.second->stackPointer;
        delete kv.second;
      }
    allThreads->clear ();

    delete allThreads;
    delete blockedID;
    delete IDQueue;
  }

  int terminateThread (int tid)
  {
    Thread *curThread = getThreadById (tid);
    if(curThread == nullptr){
      return -1;
    }
    curThread->status = TERMINATED;
    return 0;
  }

  void setRunning (int tid);
  unsigned long count(){
      return allThreads->size();
  }
};
#endif



