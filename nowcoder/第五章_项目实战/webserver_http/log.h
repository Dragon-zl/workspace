#ifndef LOG_H
#define LOG_H

#include "blockqueue.h"
#include <iostream>
#include <stdio.h>
using namespace std;

class Log{

    public:
    static Log * get_instance(){
        static Log instance;
        return &instance;
    }
    static void * flush_log_thread(void *args){
        Log :: get_instance()->async_write_log();
    }
    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int close_log, 
            int log_buf_size = 8192, int split_lines = 5000000, 
            int max_queue_size = 0);

    private:
    locker My_mutex;            //互斥锁
    FILE * My_fp;               //打开log的文件指针
    static int My_close_log;    //关闭日志
    int My_log_buf_size;        //日志缓冲区大小
    char *  My_buf;             
    int My_split_lines;         //日志最大行数
    blockqueue<string> * My_log_queue; //阻塞队列
    void * async_write_log(){
        string single_log;
        //从阻塞队列中取出一个日志 string ，写入文件
        while( My_log_queue -> pop(single_log)){
            //出队
            //加锁
            My_mutex.lock();
            fputs( single_log.c_str() , My_fp);
            My_mutex.unlock();
        }
    }
};



#endif



















