
#include <pthread.h> // for pthreads and mutex
#include <stdbool.h>
#include <stdio.h>  // for printf and NULL
#include <stdlib.h> // for malloc and free
#include <unistd.h> // for sleep

// TODO: change cond variable with semaphore + fix close channel 
// maybe a simpler version of rundevo pattern 
typedef struct {
    int data;
    bool has_data;
    bool closed;
    int number_of_waiting_read;

    pthread_mutex_t lock;
    pthread_cond_t ready_to_write;
    pthread_cond_t ready_to_read;
} Channel;

void Send(Channel *c, int v) {
    pthread_mutex_lock(&c->lock);
    while (c->number_of_waiting_read == 0 && !c->closed) {
        pthread_cond_wait(&c->ready_to_write, &c->lock);
    }
    if (c->closed) {
        printf("Channel is closed. Cannot send data.\n");
        pthread_mutex_unlock(&c->lock);
        return;
    }
    c->data = v;
    c->has_data = true;
    pthread_cond_signal(&c->ready_to_read);
    while (!c->closed && c->has_data) {
        pthread_cond_wait(&c->ready_to_write, &c->lock);
    }
    pthread_mutex_unlock(&c->lock);
}

int Recv(Channel *c) {
    pthread_mutex_lock(&c->lock);

    c->number_of_waiting_read++;
    pthread_cond_signal(&c->ready_to_write);
    while (!c->has_data && !c->closed) {
        pthread_cond_wait(&c->ready_to_read, &c->lock);
    }
    c->number_of_waiting_read--;
    if (c->closed) {
        printf("Channel is closed. Cannot receive data.\n");
        pthread_mutex_unlock(&c->lock);
        return -1; // or some other error value
    }
    int v = c->data;
    c->has_data = false;
    pthread_cond_signal(&c->ready_to_write);
    pthread_mutex_unlock(&c->lock);
    return v;
}

void close_channel(Channel *c) {
    pthread_mutex_lock(&c->lock);
    c->closed = true;
    pthread_cond_broadcast(&c->ready_to_read);
    pthread_cond_broadcast(&c->ready_to_write);
    pthread_mutex_unlock(&c->lock);
}

bool is_channel_closed(Channel *c) {
    pthread_mutex_lock(&c->lock);
    bool closed = c->closed;
    pthread_mutex_unlock(&c->lock);
    return closed;
}
Channel *make_channel(int cap) {
    Channel *c = (Channel *)malloc(sizeof(Channel));
    c->closed = false;
    c->has_data = false;
    c->number_of_waiting_read = 0;
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->ready_to_write, NULL);
    pthread_cond_init(&c->ready_to_read, NULL);
    return c;
}

typedef struct {
    int producer_id;
    Channel *c;
} producerOpts;

typedef struct {
    int consumer_id;
    Channel *c;
} consumerOpts;

void *producer(void *ops) {
    producerOpts *opts = (producerOpts *)ops;
    Channel *ch = opts->c;
    int id = opts->producer_id;

    for (int i = 0; i < 10; i++) {
        sleep(1);
        printf("[%d] is start sending %d.\n", id, i);
        Send(ch, i);
        printf("[%d] Sending is done.\n", id);
    }

    return NULL;
}
void *consumer(void *ops) {
    consumerOpts *opts = (consumerOpts *)ops;
    Channel *ch = opts->c;
    int id = opts->consumer_id;

    while (!is_channel_closed(ch)) {
        printf("[%d] Start Recving id:...\n", id);
        int v = Recv(ch);
        printf("[%d] Recving is done %d \n", id, v);
    }
    return NULL;
}

void *closer(void *opts) {
    Channel *c = (Channel *)opts;
    sleep(5);
    close_channel(c);
    printf("channel is closed");
    return NULL;
}

const int NUM_PRODUCERS = 1;
const int NUM_CONSUMERS = 3;

int run_cond_var_2(void) {

    pthread_t consumerThreads[NUM_CONSUMERS], producerThreads[NUM_PRODUCERS];
    Channel *c = make_channel(0);

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
