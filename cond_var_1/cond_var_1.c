#include "cond_var_1.h"
#include <pthread.h> // for pthreads and mutex
#include <stdbool.h>
#include <stdio.h>  // for printf and NULL
#include <stdlib.h> // for malloc and free
#include <unistd.h> // for sleep
/*
 In multi-threaded programs, it is often useful for a thread to wait for
    some condition to become true before proceeding. The simple approach,
    of just spinning until the condition becomes true, is grossly inefficient
    and wastes CPU cycles, and in some cases, can be incorrect
*/

pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int number_of_works_to_do = 0;

void *worker(void *ops) {
    Options *o = (Options *)ops;
    printf("Worker %d started...\n", o->thread_id);

    pthread_mutex_lock(&lock);
    // WE DO NEED TO WHILE AND CHECK THE CONDITION
    // because when it wakes up for a signal it might, there might be
    // another thread which has woke up a bit sooner and changed the state
    while (number_of_works_to_do == 0 /*while it should be waiting...*/) {
        // wait will:
        //      it assumes the lock is hold
        //      it will release the lock and put the thread to sleep
        //      (*atomically*) wait(park the thread using os scheduler apis)
        //
        //      when the thread wake up it re-aquire the lock
        pthread_cond_wait(&cond_var, &lock);
    }
    number_of_works_to_do--;
    printf("Thread %d: has done a work %d\n", o->thread_id,
           number_of_works_to_do);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void sleep_counddown(int n) {
    for (int i = 0; i < n; i++) {
        printf("count down ... %d ...\n", n - i);
        sleep(1);
    }
}

void *boss(void *_) {

    for (int i = 0; i < 6; i++) {
        sleep(1);
        pthread_mutex_lock(&lock);
        number_of_works_to_do += 2;
        printf("added two works\n");
        pthread_cond_broadcast(&cond_var);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int run_cond_var_simple(void) {
    pthread_cond_init(&cond_var, NULL);
    pthread_t threads[10] = {0};
    pthread_t t;
    pthread_create(&t, NULL, boss, NULL);
    for (int i = 0; i < 10; i++) {
        Options *s = (Options *)malloc(sizeof(Options));
        s->thread_id = i;
        pthread_create(&threads[i], NULL, &worker, (void *)s);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(t, NULL);

    return 0;
}
