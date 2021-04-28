#include"util.h"

#include<signal.h>
#include<cstring>
#include<fcntl.h>

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