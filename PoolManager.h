
#ifndef OS_EX2_POOLMANAGER_H
#define OS_EX2_POOLMANAGER_H

#include "Thread.h"
#include "uthreads.h"
#include "Timer.h"
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <csignal>

#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define TERMINATED 4
#define MAIN_THREAD_TID 0
using namespace std;

class PoolManager {

  //fields:
 private:
  set<int> *blockedID;
  std::map<int, Thread *> *allThreads;
  std::queue<int> *IDQueue;
  int counter;
  static Timer *_timer;


 public:
  static Thread *curRunning;

  //Methods
  PoolManager (Timer *timer)
  {
    counter = 1;
    curRunning = nullptr;
    blockedID = new set<int> ();
    allThreads = new map<int, Thread *> ();
    IDQueue = new queue<int> ();
    _timer = timer;
  }

  void initMainThread ()
  {
      Thread *newTread = new Thread ();
      newTread->stackPointer = new char(1);
      newTread->id = MAIN_THREAD_TID;
      newTread->status = READY;
      sigsetjmp(newTread->env, 1);
      sigemptyset (&(newTread->env)->__saved_mask);
      allThreads->insert (pair<int, Thread *> (newTread->id, newTread));
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
  }

  void finalTerminate (Thread *thread)
  {
    if (thread->duplicate == 1)
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
    int candidateId = IDQueue->front ();
    (*IDQueue).pop ();
    Thread *candidate = getThreadById (candidateId);
    candidate->duplicate--;//TODO לוודא שקורה פעם אחת וזהו
    while ((candidate->status != READY) || (candidate->duplicate != 1))
      {
        if (candidate->status == TERMINATED)
          {
            finalTerminate (candidate);
          }
        if ((candidate->status == BLOCKED) || (candidate->status == READY))
          {
            candidate->duplicate--;
          }
        candidateId = IDQueue->front ();
        (*IDQueue).pop ();
        candidate = getThreadById (candidateId);
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
    getThreadById (tid)->status = BLOCKED;
    auto ret = blockedID->insert (tid);
    return ret.second;
  }

  void preemptedThread ()
  {
    curRunning->status = READY;
    IDQueue->push (curRunning->id);
    curRunning->duplicate++;
  }

  int resumeThread (int tid)
  {
    auto res = blockedID->find (tid);
    if (res != blockedID->end ())
      {
        blockedID->erase (tid);
        IDQueue->push (tid);
        Thread *curThread = getThreadById (tid);
        curThread->status = READY;
        curThread->duplicate++;
        return 0;
      }
      //Thread not exist
    else
      {
        return 1;
      }
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

  void setRunning (int tid)
  {
    Thread *curThread = getThreadById (tid);
    curThread->status = RUNNING;
    curRunning = curThread;
  }
  unsigned long count(){
      return allThreads->size();
  }
};
#endif



