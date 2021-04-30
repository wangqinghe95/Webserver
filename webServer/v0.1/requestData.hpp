#ifndef REQUESTDATA_HPP__
#define REQUESTDATA_HPP__

#include<unordered_map>
#include<queue> //priority_queue

const int STATE_PARSE_URI = 1;

const int MAX_BUFF = 4096;

enum HeadersState{
    h_start = 0,
    h_key,
    
};

struct mytimer;
struct requestData;

struct mytimer{
    mytimer(requestData *_request_data, int timeout);
    ~mytimer();

    requestData* request_data;
    size_t expired_time;
    bool deleted;

    size_t getExpTime() const;
    void clearReq();
    void setDeleted();
};

class requestData
{
private:
    int now_read_pos;
    int state;
    int h_state;
    bool keep_alive;
    int again_times;
    mytimer* timer; 

    int fd;
    std::string path;
    int epollfd;
public:
    requestData(/* args */);
    requestData(int _epollfd, int _fd, std::string _path);
    ~requestData();

    void setFd(int _fd);
    int getFd();
    void addTimer(mytimer *mtimer);
    void seperateTimer();
    void handleRequest();
};

struct timerCmp{
    bool operator()(const mytimer *a, const mytimer *b)const;
};

#endif