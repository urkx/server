/**
 * 
 * Thread pool idea: https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 * 
 */
#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

struct tpool;
typedef struct tpool tpool_t;

// Thread function-to-execute type 
typedef void (*thread_task)(void* args);

tpool_t* tpool_create(size_t num);
void tpool_destroy(tpool_t *tp);

bool tpool_add_work(tpool_t* t, thread_task func, void* args);
void tpool_wait(tpool_t* t);


struct tpool_work
{
    thread_task func;
    void* args;
    struct tpool_work* next;
};
typedef struct tpool_work tpool_work_t;


struct tpool
{
    tpool_work_t*    work_first;
    tpool_work_t*    work_last;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_cond_t   working_cond;
    size_t           working_cnt;
    size_t           thread_cnt;
    bool             stop;
};

