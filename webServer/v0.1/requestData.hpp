#ifndef REQUESTDATA_HPP__
#define REQUESTDATA_HPP__

#include<unordered_map>
#include<queue> //priority_queue

const int STATE_PARSE_URI = 1;
const int STATE_PARSE_HEADERS = 2;
const int STATE_RECV_BODY = 3;
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;

const int MAX_BUFF = 4096;

//有请求但是读不到数据的尝试最大次数
const int AGAIN_MAX_TIMES = 200;

const int PARSE_URI_SUCCESS = 0;
const int PARSE_URI_AGAIN = -1;
const int PARSE_URI_ERROR = -2;

const int PARSE_HEADER_SUCCESS = 0;
const int PARSE_HEADER_AGAIN = -1;
const int PARSE_HEADER_ERROR = -2;

const int ANALYSIS_ERROR = -2;
const int ANALYSIS_SUCCESS = 0;

const int METHOD_POST = 1;
const int METHOD_GET = 2;

const int HTTP_10 = 1;
const int HTTP_11 = 2;

const int EPOLL_WAIT_TIME = 500;

class MimeType
{
    static pthread_mutex_t lock;
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);
public:
    static std::string getMime(const std::string &suffix);
};

enum HeadersState{
    h_start = 0,
    h_key,
    h_colon,
    h_space_after_colon,
    h_value,
    h_CR,
    h_LF,
    h_end_CR,
    h_end_LF,
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
    bool isDeleted() const;
    bool isValid();
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
    std::string content;
    int epollfd;
    int method;     // get or post
    std::string file_name;  // request file_name
    int HTTPversion;
    std::unordered_map<std::string, std::string> headers;
public:
    requestData(/* args */);
    requestData(int _epollfd, int _fd, std::string _path);
    ~requestData();

    void setFd(int _fd);
    int getFd();
    void addTimer(mytimer *mtimer);
    void seperateTimer();
    void handleRequest();
    void reset();
    void handleError(int fd, int err_num, std::string short_msg);

private:
    int parse_URI();
    int parse_headers();
    int analysisRequest();
};

struct timerCmp{
    bool operator()(const mytimer *a, const mytimer *b)const;
};

#endif