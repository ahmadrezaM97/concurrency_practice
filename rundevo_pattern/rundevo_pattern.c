
#include <errno.h>
#include <pthread.h> // for pthreads and mutex
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>  // for printf and NULL
#include <stdlib.h> // for malloc and free
#include <unistd.h> // for sleep

// TODO: writing rundevo pattern wasn't easy, I need to work on it
typedef struct {
    int data;
    bool has_data;
    pthread_cond_t *can_send;
    pthread_cond_t *can_recv;
    pthread_mutex_t *lock;
} Chan;

errno_t chan_init(Chan *c) {
    c->data = -1;
    c->has_data = false;

    c->is_data_ready = new_named_semaphore("is_full_sem", 0);
    if (c->is_data_ready == NULL) {
        return -1;
    }
    c->reader_ack = new_named_semaphore("is_empty_sem", 0);
    if (c->reader_ack == NULL) {
        return -1;
    }
    pthread_mutex_init(&c->lock, NULL);
    return 0;
}

errno_t chan_destroy(Chan *c) {
    if (sem_close(c->is_data_ready) == -1) {
        perror("sem_close is_full_sem");
        return -1;
    }
    if (sem_unlink("is_full_sem") == -1) {
        perror("sem_unlink is_full_sem");
        return -1;
    }

    if (sem_close(c->reader_ack) == -1) {
        perror("sem_close is_empty_sem");
        return -1;
    }
    if (sem_unlink("is_empty_sem") == -1) {
        perror("sem_unlink is_empty_sem");
        return -1;
    }

    pthread_mutex_destroy(&c->lock);
    return 0;
}

void chan_send(Chan *c, int v) {
    pthread_mutex_lock(c->lock);
    while (c->has_data) {
        pthread_cond_wait(c->can_send, c->lock);
    }

    c->data = v;
    c->has_data = true;
    pthread_cond_signal(c->can_recv);

    pthread_mutex_unlock(c->lock);


}

int chan_recv(Chan *c) {
    pthread_mutex_lock(c->lock);

    while (!c->has_data) {
        pthread_cond_wait(c->can_recv, c->lock);
    }

    int v = c->data;
    c->data = -1;
    c->has_data = false;

    pthread_cond_signal(c->can_send);

    pthread_mutex_unlock(c->lock);

    return v;
}

typedef struct {
    int producer_id;
    Chan *c;
} producerOpts;

typedef struct {
    int consumer_id;
    Chan *c;
} consumerOpts;

void *producer(void *ops) {
    producerOpts *opts = (producerOpts *)ops;
    Chan *ch = opts->c;
    int id = opts->producer_id;

    for (int i = 0; i < 10; i++) {
        sleep(1);
        printf("[%d] is start sending %d.\n", id, i);
        queue_buffer_send(ch, i);
        printf("[%d] Sending is done.\n", id);
    }

    return NULL;
}
void *consumer(void *ops) {
    consumerOpts *opts = (consumerOpts *)ops;
    Chan *ch = opts->c;
    int id = opts->consumer_id;

    while (!is_channel_closed(ch)) {
        printf("[%d] Start Recving id:...\n", id);
        int v = queue_buffer_recv(ch);
        printf("[%d] Recving is done %d \n", id, v);
    }
    return NULL;
}

#define NUM_PRODUCERS = 1
#define NUM_CONSUMERS = 3

int run_rundevo_pattern(void) {

    pthread_t consumerThreads[NUM_CONSUMERS], producerThreads[NUM_PRODUCERS];
    Chan *c = make_queue_buffer(0);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producerOpts *co = (producerOpts *)malloc(sizeof(producerOpts));
        co->producer_id = i;
        co->c = c;
        pthread_create(&producerThreads[i], NULL, producer, (void *)co);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumerOpts *co = (consumerOpts *)malloc(sizeof(consumerOpts));
        co->consumer_id = i;
        co->c = c;
        pthread_create(&consumerThreads[i], NULL, consumer, (void *)co);
    }

    pthread_t t;
    pthread_create(&t, NULL, closer, (void *)c);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumerThreads[i], NULL);
    }
    pthread_join(t, NULL);

    return 0;
}
