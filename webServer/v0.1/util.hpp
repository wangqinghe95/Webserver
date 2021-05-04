#ifndef UTIL_H__
#define UTIL_H__

#include<cstdlib>

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);

ssize_t readn(int fd, void *buff, size_t n);
ssize_t writen(int fd, void* buff, size_t n);

#endif