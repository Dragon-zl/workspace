#ifndef LOG_H
#define LOG_H

#include "../blockqueue.h"
#include <iostream>
#include <stdio.h>
#include <stdbool.h>
using namespace std;

class Log{

    public:
        static Log * get_instance(){
            static Log instance;
            return &instance;
        }
        static void * flush_log_thread(void *args){
            Log :: get_instance()->async_write_log();
            return NULL;
        }
        //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
        bool init(const char *file_name, int close_log, 
                int log_buf_size = 8192, int split_lines = 5000000, 
                int max_queue_size = 0);
        //写入日志
        void write_log(int level, const char *format, ...);
        void flush(void);

        static int My_close_log; //关闭日志
    private:
        Log();
        virtual ~Log();
        char log_name[128];         //log文件名
        char dir_name[128];         //路径名
        int  My_today;              //因为按天分类,记录当前时间是那一天
        locker My_mutex;            //互斥锁
        FILE * My_fp;               //打开log的文件指针
        int My_log_buf_size;        //日志缓冲区大小
        char *  My_buf;      
        long long My_count;         //日志行数记录       
        int My_split_lines;         //日志最大行数
        block_queue<string> * My_log_queue; //阻塞队列
        bool My_is_async;           //是否为同步标志位
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
            return NULL;
        }
    
};

#define LOG_DEBUG(format, ...) if(!Log::My_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(!Log::My_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(!Log::My_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(!Log::My_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}


#endif



















