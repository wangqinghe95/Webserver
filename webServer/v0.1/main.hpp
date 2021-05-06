#ifndef MAIN_H__
#define MAIN_H__

#include<sys/socket.h>  //socket
#include<netinet/in.h>  //struct sockaddr_in
#include<cstring> //bero
#include<sys/epoll.h>
#include<string>
using namespace std;

const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;

const int PORT = 8888;

const string PATH = "/";
const int TIMER_TIME_OUT = 500;

#endif