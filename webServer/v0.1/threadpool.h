#ifndef THREADPOOL_H__
#define THREADPOOL_H__

#include<pthread.h>

//线程任务
typedef struct{
    void (*function)(void*);
    void *argument;
}threadpool_task_t;

//线程池属性
struct threadpool_t{
    pthread_mutex_t lock;
    pthread_cond_t notify;
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

#endif