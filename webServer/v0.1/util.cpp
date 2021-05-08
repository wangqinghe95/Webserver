#include"util.hpp"

#include<signal.h>
#include<cstring>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<cstdio>
void handle_for_sigpipe(){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)){
        return;
    }
}

int setSocketNonBlocking(int fd){
    int flag = fcntl(fd, F_GETFL, 0);   //读取文件打开方式的标志，标志值和open调用一致
    if (-1 == flag){
        return -1;
    }

    flag |= O_NONBLOCK;
    if (-1 == fcntl(fd, F_SETFL, flag)){
        return -1;
    }
    
    return 0;
}

ssize_t writen(int fd, void* buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char*)buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0){
            if (nwritten < 0){
                if (errno == EINTR || errno == EAGAIN){
                    nwritten = 0;
                    continue;
                }
                else{
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }

    return writeSum;
}

ssize_t readn(int fd, void *buff, size_t n){
    // printf("readn\n");
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while (nleft > 0)
    {
        if (0 > (nread = read(fd, ptr, nleft))){
            if (errno == EINTR){     //read 等待输入期间，如果收到了一个信号，系统将转去处理该信号后失败类型
                nread = 0;  
            }
            else if (errno == EAGAIN){ //在非阻塞的情况下，连续做read而没有数据可读时，系统返回的错误
                return readSum;
            }
            else{
                return -1;
            }            
        }
        else if (0 == nread){
            break;
        }

        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }

    return readSum;    
}