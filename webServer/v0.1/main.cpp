#include<cstdio>
#include<iostream>

#include"util.h"
#include"epoll.h"

using namespace std;

int main(){

    //为了防止客户端进程终止，而导致服务器进程被 SIGPIPE 信号终止，因此服务器要处理 SIGPIPE 信号
    handle_for_sigpipe();

    int epoll_fd = epoll_init();
    if (0 > epoll_fd){
        perror("epoll init error");
        return 1;
    }

    
}