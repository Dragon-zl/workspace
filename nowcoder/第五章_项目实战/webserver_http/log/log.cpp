#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"
#include <stdio.h>

using namespace std;

//关闭日志
int Log :: My_close_log = 1;
Log::Log()
{
    My_count = 0;
    My_is_async = false;
}

Log::~Log()
{
    if (My_fp != NULL)
    {
        fclose(My_fp);
    }
}
bool Log :: init(const char *file_name, int close_log, 
            int log_buf_size, int split_lines, 
            int max_queue_size){
    //判断 如果设置了 max_queue_size ,则设置为异步
    if(max_queue_size >= 1){
        My_is_async = true;
        My_log_queue = new block_queue<string>(max_queue_size);
        pthread_t  tid;
        //flush_log_thread 为回调函数，这里表示创建线程异步写日志
        pthread_create(&tid , NULL , flush_log_thread , NULL);
    }
    
    My_close_log    = close_log;
    //设置日志缓冲区大小
    My_log_buf_size = log_buf_size;
    My_buf  =  new char[My_log_buf_size];
    //格式化字符串： 全部置 0 
    memset( My_buf , '\0' , My_log_buf_size);
    //设置日志的最大行数
    My_split_lines = split_lines;

    time_t  t = time(NULL);
    //获取实时时间
    struct  tm  * sys_tm = localtime(&t);
    struct  tm  My_tm = *sys_tm;

    const  char *p = strrchr(file_name , '/');
    char log_full_name[256] = {0};

    if( p == NULL){
        //格式化字符串：文件名
        snprintf( log_full_name , 255 , "%d_%02d_%02d_%s" , My_tm.tm_year + 1900 ,
                    My_tm.tm_mon + 1 , My_tm.tm_mday , file_name);
        
    }
    else{
        //给log 文件名和 路径名赋值
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, My_tm.tm_year + 1900,
                     My_tm.tm_mon + 1, My_tm.tm_mday, log_name);
    }
    My_today = My_tm.tm_mday;
    //打开 log 文件
    My_fp = fopen(log_full_name, "a");
    if (My_fp == NULL)
    {
        return false;
    }
    return true;
}
//写入日志
void Log::write_log(int level, const char *format, ...){
    struct timeval now = {0, 0};
    //获取当前时间
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    switch (level)
    {
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[erro]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }
    //写入一个log，对m_count++, m_split_lines最大行数
    My_mutex.lock();
    My_count++;
    //如果写入时的时间日期，和log文件的日期不一致 ， 或者 log文件 行数超过最大行数，则创建新的log文件
    if (My_today != my_tm.tm_mday || My_count % My_split_lines == 0) //everyday log
    {
        char new_log[256] = {0};
        //清空写入缓冲区
        fflush(My_fp);  
        fclose(My_fp);
        char tail[16] = {0};
        //格式化，形成新的 log 文件名
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        //分情况
        if (My_today != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            My_today = my_tm.tm_mday;
            My_count = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, My_count / My_split_lines);
        }
        //创建并打开新的 log 文件
        My_fp = fopen(new_log, "a");
    }
    My_mutex.unlock();
    va_list valst;
    va_start(valst, format);
    string log_str;
    My_mutex.lock();

    //写入的具体时间内容格式
    int n = snprintf(My_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

    int m = vsnprintf(My_buf + n, My_log_buf_size - 1, format, valst);

    My_buf[n + m] = '\n';
    My_buf[n + m + 1] = '\0';
    log_str = My_buf;
    My_mutex.unlock();

    //如果同步 或者 队列没满
    if (My_is_async && !My_log_queue->full())
    {
        //将要写入的字符串，入队
        My_log_queue->push(log_str);
    }
    else
    {
        My_mutex.lock();
        fputs(log_str.c_str(), My_fp);
        My_mutex.unlock();
    }
    va_end(valst);
}

void Log::flush(void)
{
    My_mutex.lock();
    //强制刷新写入流缓冲区
    fflush(My_fp);
    My_mutex.unlock();
}
