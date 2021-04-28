#include"requestData.hpp"
#include"util.h"
#include"epoll.h"
// #include<cstdio>

#include<iostream>
using namespace std;

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

requestData::~requestData()
{
}
