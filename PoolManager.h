
#include "Thread.h"
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <csignal>

#ifdef __x86_64__
typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7
using namespace std;

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address (address_t addr)
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
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}


#endif
#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

typedef void (*thread_entry_point) (void);

#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define TERMINATED 4
#define MAIN_THREAD_TID 0

class PoolManager {

  //fields:
 private:
  set<int> *blockedID;
  std::map<int, Thread *> *allThreads;
  std::queue<int> *IDQueue;
  int counter;

 public:
  static Thread *curRunning;

  //Methods
  PoolManager ()
  {
    counter = 1;
    curRunning = nullptr;
    blockedID = new set<int> ();
    allThreads = new map<int, Thread *> ();
    IDQueue = new queue<int> ();
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
    blockedID->insert (tid);
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
};




