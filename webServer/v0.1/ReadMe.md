

# 知识点记录
+ sigaction：修改信号处理动作（通常linux用来注册一个信号的捕捉函数）;
    - SIG_IGN:忽略信号的处理函数;
    - SIG_DFL:默认信号的处理函数
+ 信号：
    - SIGPIPE：当一个进程向某个已接收到 RST 的套接字执行写操作时，内核向该进程发送 SIGPIPE 信号；为了防止客户端进程终止，而导致服务器进程被 SIGPIPE 信号终止，因此服务器要处理 SIGPIPE 信号；

+ 消费者和生产者模型
    1. 当没有资源的时候，A，B两个资源都在阻塞等待资源，当有资源时，A，B都可以获得互斥锁了，
    2. 此时A比较快的获取互斥锁，然后加锁，消耗资源，然后解锁。此时B才获取的互斥锁，但是资源已经被消耗完了
    3. 那么必须要求它继续等待，等待的条件就是 使用 while
    while ( (pool->count == 0) && (!pool->shutdown))
    {
        pthread_cond_wait(&(pool->notify), &(pool->lock));
    }

+ 进程操作
    pthread_join:等待指定的线程结束，是线程同步的操作

+ socket 设置
    - setsockopt：设置socket传输的属性；其中 SO_REUSEADDR 允许重用本地地址和接口

+ linux 系统函数
    - fcntl：用来对已打开的文件描述符进行各种控制操作以改变已打开文件的各种属性
