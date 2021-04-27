#ifndef EPOLL_H__
#define EPOLL_H__

const int LISTENQ = 1024;
const int MAXEVENT = 5000;

int epoll_init();

#endif