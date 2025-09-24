#ifndef MUTEX_1_H
#define MUTEX_1_H

#include "pthread.h"

typedef struct {
  int counter;
  pthread_mutex_t counter_mutex;
} dataType;

int run_mutex_simple_counter(void);

#endif // MUTEX_1_H

