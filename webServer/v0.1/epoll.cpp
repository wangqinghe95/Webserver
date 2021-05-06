#include"epoll.hpp"
#include<sys/epoll.h>
#include<cstdio>

struct epoll_event* events;

int epoll_init()
{
    int epoll_fd = epoll_create(LISTENQ);
    if (-1 == epoll_fd){
        return -1;
    }
    events = new epoll_event[5000];
    return epoll_fd;
}

int epoll_add(int epoll_fd, int fd, void *request, __uint32_t events)
{
    struct epoll_event event;
    event.data.ptr = request;   //event.data.fd 监视的是文件描述符，event.data.ptr 把客户连接注册到 request 指针指向的地址
    event.events = events;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0){
        perror("epoll_add error");
        return -1;
    }

    return 0;
}

int my_epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout)
{
    int ret_count = epoll_wait(epoll_fd, events, max_events, timeout);
    if (0 > ret_count){
        perror("epoll wait error");
    }
    return ret_count;
}

int epoll_mod(int epoll_fd, int fd, void *request, __uint32_t events)
{
    struct epoll_event event;
    event.data.ptr = request;
    event.events = events;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0){
        perror("epoll_mod error");
        return -1;
    }
    return 0;
}