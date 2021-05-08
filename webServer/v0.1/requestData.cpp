#include"requestData.hpp"
#include"util.hpp"
#include"epoll.hpp"
// #include<cstdio>

#include<iostream>
#include<cstring>
#include<sys/time.h>
#include<sys/epoll.h>
#include<sys/stat.h>
#include<sys/fcntl.h>
#include<sys/mman.h>
#include<unistd.h>

// #include<opencv/cv.h>

using namespace std;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;  //等同于 pthread_mutex_init
priority_queue<mytimer*, deque<mytimer*>, timerCmp> myTimerQueue;

pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
std::unordered_map<std::string, std::string> MimeType::mime;

std::string MimeType::getMime(const std::string &suffix)
{
    if (mime.size() == 0){
        pthread_mutex_lock(&lock);
        if (mime.size() == 0){
            mime[".html"] = "text/html";
            mime[".avi"] = "video/x-msvideo";
            mime[".bmp"] = "image/bmp";
            mime[".c"] = "text/plain";
            mime[".doc"] = "application/msword";
            mime[".gif"] = "image/gif";
            mime[".gz"] = "application/x-gzip";
            mime[".htm"] = "text/html";
            mime[".ico"] = "application/x-ico";
            mime[".jpg"] = "image/jpeg";
            mime[".png"] = "image/png";
            mime[".txt"] = "text/plain";
            mime[".mp3"] = "audio/mp3";
            mime[".default"] = "text/html";
        }
        pthread_mutex_unlock(&lock);
    }
    if (mime.find(suffix) == mime.end()){
        return mime["default"];
    }
    else{
        return mime[suffix];
    }
}

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

void requestData::handleError(int fd, int err_num, std::string short_msg){
    short_msg = " " + short_msg;
    char send_buff[MAX_BUFF] = {0};
    string body_buff, header_buff;
    body_buff += "<html><title>TKeed Error</title>";
    body_buff += "<body backcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-type: text/html\r\n";
    header_buff += "Connection: close\r\n";
    header_buff += "Content-length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "\r\n";
    sprintf(send_buff, "%s", header_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
}

void requestData::reset()
{
    again_times = 0;
    content.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    keep_alive = false;
}

void requestData::handleRequest(){
    char buff[MAX_BUFF] = {0};
    bool isError = false;
    while (true)
    {
        // cout << "read fd =  " << fd << endl;
        int read_num = readn(fd, buff, MAX_BUFF);
        cout << "read_num = " << read_num << endl;
        // cout << "buff = " << buff << endl; 
        if (0 > read_num){
            perror("read error");
            isError = true;
            break;
        }
        else if (0 == read_num){
            //有请求但是读不到数据，可能是 request abort 
            // 或者是网络请求还未到达
            perror("readnum == 0");
            if (errno = EAGAIN){
                if (again_times > AGAIN_MAX_TIMES){
                    isError = true;
                }
                else {
                    again_times++;
                }                
            } 
            else if (errno != 0){
                isError = true;
            }
            break;
        }
        string now_read(buff, buff+read_num);
        content += now_read;
        cout << "state " << state << endl;
        if (state == STATE_PARSE_URI){
            int flag = parse_URI();
            if (flag == PARSE_URI_AGAIN){
                break;
            }
            else if (flag == PARSE_URI_ERROR){
                perror("parse URI error");
                isError = true;
                break;
            }
        }
        if (state == STATE_PARSE_HEADERS){
            int flag = parse_headers();
            if (flag == PARSE_HEADER_AGAIN){
                break;
            }
            else if (flag == PARSE_HEADER_ERROR){
                perror("parse headers error");
                isError = true;
                break;
            }

            if (method == METHOD_POST){
                state = STATE_RECV_BODY;
            }
            else{
                state = STATE_ANALYSIS;
            }
        }
        if (state == STATE_RECV_BODY){
            int content_length = -1;
            if (headers.find("Content-length") != headers.end()){
                content_length = stoi(headers["Content-length"]);
            }
            else{
                isError = true;
                break;
            }

            if (content.size() < content_length){
                continue;
            }
            state = STATE_ANALYSIS;
        }
        if (state == STATE_ANALYSIS){
            int flag = analysisRequest();
            if (0 > flag){
                isError = true;
                break;
            }
            else if (flag == ANALYSIS_SUCCESS){
                state = STATE_FINISH;
                break;
            }
            else{
                isError = true;
                break;
            }
        }
    }

    if (isError){
        delete this;
        return;
    }

    // 加入 epoll继续
    if (state == STATE_FINISH){
        if (keep_alive){
            printf("keep_alive OK\n");
            reset();
        }
        else{
            delete this;
            return ;
        }
    }

    //需要加入时间信息，避免刚加进去，下个 in 出发进来出发分离失败
    //新增时间信息
    pthread_mutex_lock(&qlock);
    mytimer *mtimer = new mytimer(this, 500);
    timer = mtimer;
    myTimerQueue.push(mtimer);
    pthread_mutex_unlock(&qlock);

    __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_mod(epollfd, fd, static_cast<void*>(this), _epo_event);
    if (ret < 0){
        delete this;
        return;
    }
    
}

int requestData::analysisRequest()
{
    printf("analysisRequest\n");
    if (method == METHOD_POST){
        char header[MAX_BUFF] = {0};
        sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        if (headers.find("Connection") != headers.end() && headers["Connecrion"] == "keep-alive"){
            keep_alive = true;
            sprintf(header, "%sConnection: keep-alive\r\n", header);
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
        }

        char *send_content = "I have received this.";

        sprintf(header, "%sContent-length: %zu\r\n", header, strlen(send_content));
        sprintf(header, "%s\r\n", header);

        size_t send_len = (size_t)writen(fd, header, strlen(header));
        if (send_len != strlen(send_content)){
            perror("Send content failed");
            return ANALYSIS_ERROR;
        }

        cout << "content size = " << content.size() << endl;
        vector<char> data(content.begin(), content.end());
        // Mat test = imdecode(data,)
        return ANALYSIS_SUCCESS;
    }
    else if (method == METHOD_GET) {
        char header[MAX_BUFF] = {0};
        sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
        if (headers.find("Connection") != headers.end()
            && headers["Connection"] == "keep-alive"){
            keep_alive = true;
            sprintf(header, "%sConnection: keep-alive\r\n", header);
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);            
        }

        int dot_pos = file_name.find('.');
        const char *file_type;
        if (dot_pos < 0){
            file_type = MimeType::getMime("default").c_str();
        }
        else{
            file_type = MimeType::getMime(file_name.substr(dot_pos)).c_str();
        }

        struct stat sbuf;
        if (stat(file_name.c_str(), &sbuf) < 0){
            handleError(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }

        sprintf(header, "%sContent-type: %s\r\n", header, file_type);
        sprintf(header, "%sContent-length: %ld\r\n", header, sbuf.st_size);
        sprintf(header, "%s\r\n", header);
        size_t send_len = (size_t)writen(fd, header, strlen(header));
        if (send_len != strlen(header)){
            perror("Send header failed");
            return ANALYSIS_ERROR;
        }

        int src_fd = open(file_name.c_str(), O_RDONLY, 0);
        char *src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        close(src_fd);

        send_len = writen(fd, src_addr, sbuf.st_size);
        if (send_len != sbuf.st_size){
            perror("Send file failed");
            return ANALYSIS_ERROR;
        }
        
        munmap(src_addr, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    else
    {
        return ANALYSIS_ERROR;
    }    
}

int requestData::parse_headers()
{
    string &str = content;

    //有限状态机的标识位
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;

    bool notFinish = true;

    //处理的方法应该是有限状态机
    for (int i = 0; i < str.size() && notFinish; ++i){
        switch (h_state)
        {
        case h_start:
        {
            if (str[i] == '\n' || str[i] == '\r'){
                break;
            }
            h_state = h_key;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        case h_key:
        {
            if (str[i] == ':'){
                key_end = i;
                if (key_end - key_start <= 0){
                    return PARSE_HEADER_ERROR;
                }
                h_state = h_colon;
            }
            else if (str[i] == '\n' || str[i] == '\r'){
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case h_colon:
        {
            if (str[i] == ' '){
                h_state = h_space_after_colon;
            }
            else{
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case h_space_after_colon:
        {
            h_state = h_value;
            value_start = i;
            break;
        }
        case h_value:
        {
            if (str[i] == '\r'){
                h_state = h_CR;
                value_end = i;
                if (value_end - value_start <= 0){
                    return PARSE_HEADER_ERROR;
                }
            }
            else if (i - value_start > 255){
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case h_CR:
        {
            if (str[i] == '\n'){
                h_state = h_LF;
                string key(str.begin() + key_start, str.begin() + key_end);
                string value(str.begin() + value_start, str.begin() + value_end);
                headers[key] = value;
                now_read_line_begin = i;
            }
            else{
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case h_LF:
        {
            if (str[i] == '\r'){
                h_state = h_end_CR;
            }
            else{
                key_start = i;
                h_state = h_key;
            }
            break;
        }
        case h_end_CR:
        {
            if (str[i] == '\n'){
                h_state = h_end_LF;
            }
            else{
                return PARSE_HEADER_ERROR;
            }
            break;
        }
        case h_end_LF:
        {
            notFinish = false;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        }
    }

    if (h_state == h_end_LF){
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }

    str = str.substr(now_read_line_begin);
    return PARSE_URI_AGAIN;
}

int requestData::parse_URI()
{   
    string &str = content;

    //读到完整的请求行时再开始解析请求；
    int pos = str.find('\r', now_read_pos);
    if ( 0 > pos){
        return PARSE_URI_AGAIN;
    }

    //去掉请求行，节省内存空间
    string request_line = str.substr(0, pos);
    if (str.size() > pos + 1){
        str = str.substr(pos+1);
    }
    else{
        str.clear();
    }

    //处理请求行-调用方法
    pos = request_line.find("GET");
    if (0 > pos){
        pos = request_line.find("POST");
        if (0 > pos){
            return PARSE_URI_ERROR;
        }
        else{
            method = METHOD_POST;
        }
    }
    else{
        method = METHOD_GET;
    }

    //处理请求行-文件名（网页的路由显示名）
    pos = request_line.find("/", pos);
    if (0 > pos){
        return PARSE_URI_ERROR;
    }
    else{
        int _pos = request_line.find(' ', pos);
        if (0 > _pos){
            return PARSE_URI_ERROR;
        }
        else{
            if (_pos - pos > 1){
                file_name = request_line.substr(pos+1, _pos-pos-1);
                int __pos = file_name.find("?");
                if ( 0 > __pos ){
                    file_name = file_name.substr(0, __pos);
                }
            }
            else{
                file_name = "index.html";
            }
        }
        pos = _pos;
    }

    //HTTP 版本号
    pos = request_line.find("/", pos);
    if (0 > pos){
        return PARSE_URI_ERROR;
    }
    else{
        if (request_line.size() - pos <= 3){
            return PARSE_URI_ERROR;
        }
        else{
            string ver = request_line.substr(pos+1, 3);
            if (ver == "1.0"){
                HTTPversion = HTTP_10;
            }
            else if (ver == "1.1"){
                HTTPversion = HTTP_11;
            }
            else{
                return PARSE_URI_ERROR;
            }        
        }
    }

    cout << "request_line" << request_line; 
    state = STATE_PARSE_HEADERS;
    return PARSE_URI_SUCCESS;
}

mytimer::mytimer(requestData *_request_data, int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

mytimer::~mytimer()
{
    if (request_data != NULL){
        delete request_data;
        request_data = NULL;
    }
}

size_t mytimer::getExpTime() const
{
    return expired_time;
}

bool timerCmp::operator()(const mytimer *a, const mytimer *b)const
{
    return a->getExpTime() > b->getExpTime();
}

void mytimer::clearReq()
{
    request_data = nullptr;
    this->setDeleted();
}

void mytimer::setDeleted()
{
    deleted = true;
}

bool mytimer::isDeleted() const
{
    return deleted;
}
bool mytimer::isValid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
    if (temp < expired_time){
        return true;
    }
    else{
        this->setDeleted();
        return false;
    }
}
