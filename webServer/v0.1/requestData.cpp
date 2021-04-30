#include"requestData.hpp"
#include"util.h"
#include"epoll.h"
// #include<cstdio>

#include<iostream>
#include<sys/time.h>

using namespace std;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;  //等同于 pthread_mutex_init
priority_queue<mytimer*, deque<mytimer*>, timerCmp> MyTimerQueue;

requestData::requestData()
        :now_read_pos(0)
        ,state(STATE_PARSE_URI)
        ,h_state(h_start)
        ,keep_alive(false)
        ,again_times(0)
        ,timer(nullptr)
{
    cout << "requestData constructed!" << endl;
}

requestData::requestData(int _epollfd, int _fd, std::string _path)
            : now_read_pos(0)
            , state(STATE_PARSE_URI)
            , h_state(h_start)
            , keep_alive(false)
            , again_times(0)
            , timer(nullptr)
            , path(_path)
            , fd(_fd)
            , epollfd(_epollfd)
{

}

requestData::~requestData()
{
}

void requestData::setFd(int _fd){
    fd = _fd;
}

int requestData::getFd(){
    return fd;
}

void requestData::addTimer(mytimer *mtimer){
    if (timer == NULL){
        timer = mtimer;
    }
}

void requestData::seperateTimer(){
    if (timer){
        timer->clearReq();
        timer = NULL;
    }
}

void requestData::handleRequest(){
    char buff[MAX_BUFF] = {0};
    bool isError = false;
    while (true)
    {
        int read_num = readn(fd, buff, MAX_BUFF);   //2
    }
    
}

mytimer::mytimer(requestData *_request_data, int timeout){
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

size_t mytimer::getExpTime() const{
    return expired_time;
}

bool timerCmp::operator()(const mytimer *a, const mytimer *b)const{
    return a->getExpTime() > b->getExpTime();
}

void mytimer::clearReq(){
    request_data = nullptr;
    this->setDeleted();
}

void mytimer::setDeleted(){
    deleted = true;
}
