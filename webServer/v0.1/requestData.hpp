#ifndef REQUESTDATA_HPP__
#define REQUESTDATA_HPP__

const int STATE_PARSE_URI = 1;

enum HeadersState{
    h_start = 0,
    h_key,
    
};

struct mytimer{
    mytimer(requestData *_request_data, int timeout);
    ~mytimer();
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
public:
    requestData(/* args */);
    ~requestData();
};



#endif