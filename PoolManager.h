//
// Created by shirayarhi on 30/03/2022.
//
#include "Location.cpp"
#include <vector>
#include <set>
#include <map>
#include <queue>
#include "Thread.h"
#include "uthreads.h"
#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define TERMINATED 4

class PoolManager {

  //fields:
 private:
  std::set<int> blockedID;
  std::map<int, Thread*> allThreads;
  std::queue<int> IDQueue;
  int counter;
  Thread *curRunning;

 public:

  //Methods
  PoolManager ()
  {
    counter = 0;
  }

  void addThread (char *stack, thread_entry_point entry_point)
  {
    Thread *newTread = new
        Thread ();
    newTread->id = counter;
    newTread->status = READY;
    address_t sp = (address_t) stack + STACK_SIZE - sizeof (address_t);
    address_t pc = (address_t) entry_point;
    sigsetjmp(newTread->env, 1);
    ((newTread->env)->__jmpbuf)[JB_SP] = translate_address (sp);
    ((newTread->env)->__jmpbuf)[JB_PC] = translate_address (pc);
    sigemptyset (&(newTread->env)->__saved_mask);
    counter++;
    allThreads.insert (std::pair<int, Thread*> (newTread->id, newTread));
  }

  Thread *nextAvailableReady ()
  {
    int candidateId = IDQueue.front ();
    IDQueue.pop ();
    Thread *candidate = getThreadById (candidateId);
    while ((candidate->status != READY) || (candidate->duplicate != 1))
      {
        if (candidate->status == TERMINATED)
          {
            allThreads.erase (candidateId);
          }
        if (candidate->status == BLOCKED)
          {
            candidate->duplicate--; // מעיפים את התרד שעשו לו בלוק
          }
      }
    return candidate;
  }

  Thread *getThreadById (int id)
  {
    auto res = allThreads.find (id);
    if (res != allThreads.end ())
      {
        return &(res->second);
      }
    else
      {
        return nullptr;
      }
  }

  int blockThread (int tid)
  {
    getThreadById (tid)->status = BLOCKED;
    blockedID.insert (tid);
  }

  int resumeThread (int tid)
  {
    auto res = blockedID.find (tid);
    if (res != blockedID.end ())
      {
        blockedID.erase (tid);
        IDQueue.push (tid);
        Thread *curThread = getThreadById (tid);
        curThread->status = READY;
        curThread->duplicate++;
        return 0;
      }
      //Thread not blocked
    else
      {
        return -1;
      }
  }

  void terminateThread (int tid)
  {
    Thread *curThread = getThreadById (tid);
    curThread->status = TERMINATED;
  }

};
