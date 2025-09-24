#include <pthread.h> // for pthreads and mutex
#include <stdbool.h>
#include <stdio.h>  // for printf and NULL
#include <stdlib.h> // for malloc and free
#include <unistd.h> // for sleep

typedef struct {
    int len;
    int cap;
    int data[100];
    pthread_mutex_t lock;
    pthread_cond_t has_data;
    pthread_cond_t is_full;
} queue_buffer;

void queue_buffer_send(queue_buffer *c, int v) {
    pthread_mutex_lock(&c->lock);

    while (c->len == c->cap) {
        pthread_cond_wait(&c->is_full, &c->lock);
    }
    c->data[c->len] = v;
    c->len++;
    pthread_cond_broadcast(&c->has_data);

    pthread_mutex_unlock(&c->lock);
}

int queue_buffer_recv(queue_buffer *c) {
    pthread_mutex_lock(&c->lock);

    while (c->len == 0) {
        pthread_cond_wait(&c->has_data, &c->lock);
    }
    int v = c->data[c->len - 1];
    c->len--;
    pthread_cond_broadcast(&c->is_full);

    pthread_mutex_unlock(&c->lock);
    return v;
}

queue_buffer *make_queue_buffer(int cap) {
    queue_buffer *c = (queue_buffer *)malloc(sizeof(queue_buffer));
    c->cap = cap;
    c->len = 0;
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->has_data, NULL);
    pthread_cond_init(&c->is_full, NULL);
    return c;
}

typedef struct {
    int producer_id;
    queue_buffer *c;
} producerOpts;

typedef struct {
    int consumer_id;
    queue_buffer *c;
} consumerOpts;

void *producer(void *ops) {
    producerOpts *opts = (producerOpts *)ops;
    queue_buffer *ch = opts->c;
    int id = opts->producer_id;

    printf("[%d] Start sending ... %d\n", id, id);
    queue_buffer_send(ch, id);
    printf("[%d] Sending is done.\n", id);

    return NULL;
}
void *consumer(void *ops) {
    consumerOpts *opts = (consumerOpts *)ops;
    queue_buffer *ch = opts->c;
    int id = opts->consumer_id;

    printf("[%d] Start Recving id:...\n", id);
    sleep(2);
    int v = queue_buffer_recv(ch);
    printf("[%d] Recving id is done %d \n", id, v);

    return NULL;
}

const int NUM_PRODUCERS = 3;
const int NUM_CONSUMERS = 3;

int run_cond_var_2(void) {
    pthread_t consumerThreads[NUM_CONSUMERS], producerThreads[NUM_PRODUCERS];
    queue_buffer *c = make_queue_buffer(1);

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

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumerThreads[i], NULL);
    }

    return 0;
}
