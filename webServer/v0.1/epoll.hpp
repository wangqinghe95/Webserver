#ifndef EPOLL_H__
#define EPOLL_H__

const int LISTENQ = 1024;
const int MAXEVENTS = 5000;

int epoll_init();

int epoll_add(int epoll_fd, int fd, void *request, __uint32_t events);

int epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout);

int epoll_mod(int epoll_fd, int fd, void *request, __uint32_t events);

#endif