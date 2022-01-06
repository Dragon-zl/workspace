#ifndef LOG_H
#define LOG_H

#include<stdio.h>
#include <iostream>
#include "block_queue.h"
using namespace std;
class Log{
    public:
        static Log * get_instance(){
            static Log  instance;
            return &instance;
        }
        static void * flush_log_thread(void *args){
            //调用异步 写入日志函数
            Log :: get_instance()->async_write_log();
        }
        //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
        bool init(const char *file_name, int close_log, 
            int log_buf_size = 8192, int split_lines = 5000000, 
            int max_queue_size = 0);
        

    private:
        //是否异步标志位
        bool My_is_async;   
        //阻塞队列
        block_queue<string> * My_log_queue;    

};


#endif 