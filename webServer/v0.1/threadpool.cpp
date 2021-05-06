#include"threadpool.hpp"

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags){
    // printf("threadpool_create\n");
    threadpool_t *pool;

    do
    {
        if (thread_count <= 0 || thread_count > MAX_THREADS 
            || queue_size <= 0 || queue_size > MAX_QUEUE)
        {
            return NULL;
        }

        if ((pool = (threadpool_t*)malloc(sizeof(threadpool_t))) == NULL){
            break;
        }

        //初始化线程池
        pool->thread_count = 0;
        pool->queue_size = queue_size;
        pool->head = pool->tail = pool->count = 0;
        pool->shutdown = pool->started = 0;

        //分配线程和任务队列
        pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
        pool->queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t) * queue_size);

        //初始化互斥锁和条件变量
        if ( (pthread_mutex_init(&(pool->lock), NULL) != 0) ||
             (pthread_cond_init(&(pool->notify), NULL) != 0) ||
             (pool->threads == NULL) || (pool->queue == NULL))
        {
            break;
        }

        //线程运行
        for (int i = 0; i < thread_count; i++){
            if (pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0){
                threadpool_destory(pool, 0);
                return NULL;
            }
            pool->thread_count++;
            pool->started++;
        }

        return pool;
    } while (false);

    if (pool != NULL){
        threadpool_free(pool);
    }

    return NULL;    
}

int threadpool_add(threadpool_t *pool, void (*function)(void*), void* argument, int flags){
    // printf("threadpool_add\n");
    int err = 0;
    
    if (NULL == pool || NULL == function){
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&(pool->lock)) != 0){
        return THREADPOOL_LOCK_FAILURE;
    }

    int next = (pool->tail + 1) % pool->queue_size;
    do
    {
        if (pool->count == pool->queue_size){
            err = THREADPOOL_QUEUE_FULL;
            break;
        }

        if(pool->shutdown){
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        if (pthread_cond_signal(&(pool->notify)) != 0){
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
    } while (false);

    if (pthread_mutex_unlock(&pool->lock) != 0){
        err = THREADPOOL_LOCK_FAILURE;
    }
    // printf("next = %d, err = %d\n", next, err);
    return err;    
}

int threadpool_destory(threadpool_t* pool, int flags){
    printf("Thread pool destoty !\n");

    int err = 0;

    if (NULL == pool){
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&(pool->lock)) != 0){
        return THREADPOOL_LOCK_FAILURE;
    }

    do
    {
        //关闭线程池？
        if (pool->shutdown){
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? 
            graceful_shutdown : immedidate_shutdown;

        //等待工作线程结束后回收资源
        for (int i = 0; i < pool->thread_count; ++i){
            if (pthread_join(pool->threads[i], NULL) != 0){
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    } while (false);

    //当所有的线程任务都执行完后才释放线程池
    if (!err){
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool){
    printf("threadpool_free\n");

    if (pool == NULL || pool->started > 0){
        return -1;
    }

    if (pool->threads){
        free(pool->threads);
        free(pool->queue);

        //因为我们在初始化互斥锁和条件变量后再分配线程池中线程指针的
        //所以我们确定他们是被初始化的，所以再销毁之前要先锁住
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }

    free(pool);
    return 0;
}

static void* threadpool_thread(void *threadpool){
    printf("threadpool_thread\n");

    threadpool_t *pool = (threadpool_t*)threadpool;
    threadpool_task_t task;

    for(;;){
        //加锁等待条件变量
        pthread_mutex_lock(&(pool->lock));

        //条件不足，进程需要在这里等待
        //当 pthread_cond_wait 返回时，我们就获得了互斥锁
        //当没有资源的时候，A，B两个资源都在阻塞等待资源，当有资源时，A，B可以获得互斥锁了
        //此时A比较快的获取互斥锁，然后加锁，消耗资源，然后解所，
        while ( (pool->count == 0) && (!pool->shutdown))
        {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }
        
        //什么情况时退出循环
        if ( (pool->shutdown == immedidate_shutdown) || 
             ((pool->shutdown = graceful_shutdown) && (pool->count == 0)))
        {
            break; 
        }

        //执行任务
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        //释放互斥锁
        pthread_mutex_unlock(&(pool->lock));

        //执行函数
        (*(task.function))(task.argument);
    }

    --pool->started;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return (NULL);
}