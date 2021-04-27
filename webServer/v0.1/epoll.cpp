#include"epoll.h"
#include<sys/epoll.h>

struct epoll_event* event;

int epoll_init(){
    int epoll_fd = epoll_create(LISTENQ);
    if (-1 == epoll_fd){
        return -1;
    }
    event = new epoll_event[5000];
    return epoll_fd;
}