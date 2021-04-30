#ifndef THREADPOOL_H__
#define THREADPOOL_H__

#include<pthread.h>
#include<cstdlib>
#include<cstdio>

const int THREADPOOL_GRACEFUL = 1;
const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;

const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

typedef enum{
    immedidate_shutdown = 1,
    graceful_shutdown = 2
}threadpool_shutdown_t;

//线程任务
typedef struct{
    void (*function)(void*);
    void *argument;
}threadpool_task_t;

//线程池属性
struct threadpool_t{
    pthread_mutex_t lock;   //互斥锁，进入临界去等待
    pthread_cond_t notify;  //条件变量，预防死锁
    pthread_t *threads;
    threadpool_task_t *queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
    int started;  
};

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags);

int threadpool_add(threadpool_t *pool, void (*function)(void*), void* argument, int flags);

int threadpool_destory(threadpool_t* pool, int flags);

int threadpool_free(threadpool_t *pool);

static void* threadpool_thread(void *threadpool);   //线程执行函数

#endif