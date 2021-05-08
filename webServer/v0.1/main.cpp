#include<cstdio>
#include<iostream>

#include"util.hpp"
#include"epoll.hpp"
#include"threadpool.hpp"
#include"main.hpp"
#include"requestData.hpp"

using namespace std;

extern struct epoll_event* events;
extern pthread_mutex_t qlock;
extern priority_queue<mytimer*, deque<mytimer*>, timerCmp> myTimerQueue;


int socket_bind_listen(int port);
//分发处理函数
void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string &path, threadpool_t* tp);

void acceptConnection(int listen_fd, int epoll_fd, const string &path);

void myHandler(void *args);

void handle_expired_event();

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
    req->setFd(listen_fd);

    epoll_add(epoll_fd, listen_fd, static_cast<void*>(req), event);

    while (true)
    {
        int events_num = my_epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        if (0 == events_num){
            continue;
        }

        // cout << "event_num : " << events_num << endl;
        handle_events(epoll_fd, listen_fd, events, events_num, PATH, threadpool);
    
        handle_expired_event();
    }
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

void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string &path, threadpool_t* tp){
    for (int i = 0; i < events_num; ++i){
        // cout << "handle_events: i = " << i << endl; 
        //获取有事件产生的描述符
        requestData* request = (requestData*)(events[i].data.ptr);
        int fd = request->getFd();
        // cout << "fd = " << fd << "listen_fd" << listen_fd << endl;
        //有事件发生的描述符为监听描述符
        if (fd == listen_fd){
            // cout << "acceptConnection" << endl;
            acceptConnection(listen_fd, epoll_fd, path);
        }
        else{
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                    || (!events[i].events & EPOLLIN)){
                cout << "error event" << endl;
                delete request;
                continue;
            }

            request->seperateTimer();
            int rc = threadpool_add(tp, myHandler, events[i].data.ptr, 0);
            // printf("rc = %d\n", rc);
        }
        
    }
}

void acceptConnection(int listen_fd, int epoll_fd, const string &path){
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));

    socklen_t client_addr_len = 0;
    int accept_fd = 0;
    while (0 < (accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)))
    {
        int ret = setSocketNonBlocking(accept_fd);
        if (0 > ret){
            perror("Set non block failed!");
            return;
        }

        requestData* req_info = new requestData(epoll_fd, accept_fd, path);

        //文件描述符可以读，边缘触发（Edge Triggered）模式，保证一个 socket 连接在任意一个时刻只被一个线程处理
        __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
        // cout << "add fd " << accept_fd << endl;
        epoll_add(epoll_fd, accept_fd, static_cast<void*>(req_info), _epo_event);

        //新增时间信息
        mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
        req_info->addTimer(mtimer);

        pthread_mutex_lock(&qlock);
        myTimerQueue.push(mtimer);
        pthread_mutex_unlock(&qlock);
    }
    
}

void myHandler(void *args)
{
    requestData *req_data = (requestData *)args;
    req_data->handleRequest();
}

void handle_expired_event()
{
    pthread_mutex_lock(&qlock);
    while (!myTimerQueue.empty())
    {
        mytimer *ptimer_now = myTimerQueue.top();
        if (ptimer_now->isDeleted()){
            myTimerQueue.pop();
            delete ptimer_now;
        }
        else if(ptimer_now->isValid() == false){
            myTimerQueue.pop();
            delete ptimer_now;
        }
        else{
            break;
        }
    }
    pthread_mutex_unlock(&qlock);    
}