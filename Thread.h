//
// Created by shirayarhi on 30/03/2022.
//

#ifndef OS_EX2_THREAD_H
#define OS_EX2_THREAD_H
#include <setjmp.h>


class Thread {

 public:
  int id;
  int status;
  sigjmp_buf env;
  int duplicate;
};


#endif //OS_EX2_THREAD_H
