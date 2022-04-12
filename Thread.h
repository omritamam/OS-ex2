

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H
#include <setjmp.h>


class Thread {

 public:
  int id;
  int status;
  sigjmp_buf env;
  int duplicate;
  int quantum;
  char* stackPointer;
  bool isSleep;
  int waitingTime;
};


#endif
