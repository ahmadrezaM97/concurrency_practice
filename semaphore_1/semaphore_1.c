#include <errno.h>
#include <fcntl.h> // O_CREAT, O_EXCL
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define THREAD_NUM 10 // don't need a semicolon here

typedef struct {
    int id;
} Options;

sem_t *sempahore_ptr;

void *worker(void *ops) {
    Options *options = (Options *)ops;

    printf("worker id %d is waiting...\n", options->id);
    while (sem_wait(sempahore_ptr) == -1 && errno == EINTR) {
        // retry if interrupted by signal
    }

    printf("worker id %d: is working\n", options->id);
    sleep(1);

    printf("worker id %d: is done\n", options->id);
    sem_post(sempahore_ptr);
    free(options);
    return NULL;
}

int run(void) {
    int worker_limit = 3;

    // unnamed semaphore
    // sem_init(&sempahore, 0, 3);
    // *** sem_init is depricated in MACOS

    // named semaphore

    const char *sem_name = "/ahmad_semaphore";
    // Clean up any stale semaphore from prior runs
    if (sem_unlink(sem_name) == -1 && errno != ENOENT) {
        perror("sem_unlink (pre-clean)");
        return 1;
    }
    // Create with initial count = worker_limit
    sempahore_ptr = sem_open(sem_name, O_CREAT | O_EXCL, 0644, worker_limit);
    if (sempahore_ptr == SEM_FAILED) {
        if (errno == EEXIST) {
            // Fallback: open existing one
            if (sempahore_ptr == SEM_FAILED) {
                perror("sem_open existing");
                return 1;
            }
        } else {
            perror("sem_open");
            return 1;
        }
    }

    pthread_t t[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++) {
        Options *option = (Options *)malloc(sizeof(Options));
        if (!option) {
            perror("malloc failed.");
        }
        option->id = i;
        if (pthread_create(&t[i], NULL, &worker, (void *)option) != 0) {
            perror("pthread_create");
            free(option);
        }
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(t[i], NULL);
    }

    // this closes your process's reference to an already opened semaphore
    // this of it like fclose for  a file; it doesn't remove the semaphore, it
    // just tells the kernel "this process is done usingit"
    if (sem_close(sempahore_ptr) == -1) {
        perror("sem_close");
    }

    if (sem_unlink(sem_name) == -1) {
        perror("sem_unlink");
    }
    return 0;
}

//  *  Initialize an unnamed semaphore (pshared==0: threads; !=0: processesvia
//  shared memory) */
// int sem_init(sem_t *sem, int pshared, unsigned int value);
//
//  Destroy an unnamed semaphore previously initialized with sem_init */
// int sem_destroy(sem_t *sem);
//
//  Decrement (wait) the semaphore; block if value == 0 */
// int sem_wait(sem_t *sem);
//
//  Try to decrement without blocking; on zero returns -1 and sets
// errno=EAGAIN */ int sem_trywait(sem_t *sem);
//
//  Wait with an absolute timeout (struct timespec *abstime); on timeout
// errno==ETIMEDOUT */ int sem_timedwait(sem_t *sem, const struct timespec
// *abstime);
//
//  Increment (post) the semaphore; wakes one waiter if present */
// int sem_post(sem_t *sem);
//
//  Get the current value (informational only — not reliable for
// synchronization) */ int sem_getvalue(sem_t *sem, int *sval);
//
//  ---------- Named (system-wide) semaphores (inter-process) ---------- */
//
//  Open or create a named semaphore; returns pointer or SEM_FAILED on error.
//    If O_CREAT used, supply mode and initial value: sem_open(name, O_CREAT,
//    mode, value) */
// sem_t *sem_open(const char *name, int oflag, ...);
//
//  Close the named semaphore descriptor in this process (doesn't remove the
//  name) */ int sem_close(sem_t *sem);
//
// // Remove the named semaphore from the system namespace (like unlink) */
// int sem_unlink(const char *name
