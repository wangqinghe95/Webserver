#include<cstdio>
#include<iostream>

#include"util.h"
#include"epoll.h"
#include"threadpool.h"
#include"main.h"
#include"requestData.hpp"

using namespace std;

int socket_bind_listen(int port);

int main(){

    //为了防止客户端进程终止，而导致服务器进程被 SIGPIPE 信号终止，因此服务器要处理 SIGPIPE 信号
    handle_for_sigpipe();

    int epoll_fd = epoll_init();
    if (0 > epoll_fd){
        perror("epoll init error");
        return 1;
    }

    threadpool_t* threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);
    int listen_fd = socket_bind_listen(PORT);
    if (0 > listen_fd){
        perror("socket bind failed");
        return 1;
    }

    if(0 > setSocketNonBlocking(listen_fd)){
        perror("set socket non block failed");
        return 1;
    }

    __uint32_t event = EPOLLIN | EPOLLET;
    requestData *req = new requestData();
}

int socket_bind_listen(int port){

    //监听端口要在一定的区间里
    if (port < 1024 || port > 65535){
        return -1;
    }

    int listen_fd = 0;
    if ( -1 == (listen_fd = socket(AF_INET, SOCK_STREAM, 0))){
        return -1;
    }

    //消除bind时“address already in use” 错误
    int optval = 1;
    if ( -1 == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))){
        return -1;
    }

    //设置服务器IP和port
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);

    if (-1 == bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        return -1;
    }

    if (-1 == listen(listen_fd, LISTENQ)){
        return -1;
    }

    return listen_fd;
}