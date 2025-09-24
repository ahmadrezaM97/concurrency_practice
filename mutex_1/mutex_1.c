#include "mutex_1.h"
#include <pthread.h>
#include <stdio.h>

void *worker(void *ops) {
    dataType *d = (dataType *)ops;

    for (int i = 0; i < 10000000; i++) {
        // pthread_mutex_lock(&d->counter_mutex);
        d->counter++;
        // pthread_mutex_unlock(&d->counter_mutex);
    }
    return NULL;
}

int run_mutex_simple_counter(void) {

    dataType d = {.counter = 0, .counter_mutex = PTHREAD_MUTEX_INITIALIZER};
    int n = 9;
    pthread_t t[9];
    for (int i = 0; i < 9; i++) {
        pthread_create(&t[i], NULL, worker, (void *)(&d));
    }
    for (int i = 0; i < 9; i++) {
        pthread_join(t[i], NULL);
    }

    printf("%d\n", d.counter);

    return 0;
}
