#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>

#include "log.h"
using namespace std;

Log::Log()
{
    m_count = 0;  //日志行数记录 归零
    m_is_async = false; //异步标志位 false
}
Log::~Log()
{
    if (m_fp != NULL)
    {
        fclose(m_fp);//关闭打开的log文件流
    }
}
//异步需要设置阻塞队列的长度，同步不需要设置
bool Log::init(const char *file_name, int close_log, 
                int log_buf_size, int split_lines, int max_queue_size)
{
    //如果设置了max_queue_size,则设置为异步
    if (max_queue_size >= 1){
        m_is_async = true;
        //创建阻塞队列
        m_log_queue = new block_queue<string>(max_queue_size);
        pthread_t tid;
        //flush_log_thread为回调函数,这里表示创建线程异步写日志
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }
    m_close_log = close_log;
    m_log_buf_size = log_buf_size;//日志缓冲区大小
    m_buf = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);//清空缓冲数组
    m_split_lines = split_lines;//日志最大行数

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);//获取当前时间，用于log文件名命名
    struct tm my_tm = *sys_tm;

    const char *p = strrchr(file_name, '/');//返回‘/’在file_name最后一次出现的位置
    char log_full_name[256] = {0};
    if (p == NULL)
    {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else
    {   //格式化 log 的文件名绝对路径
        strcpy(log_name, p + 1);
        strncpy(dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }
    m_today = my_tm.tm_mday;
    m_fp = fopen(log_full_name, "a");
    if (m_fp == NULL)
    {
        return false;
    }
    return true;
}
/*日志写入函数
  传入参数：
  level ：日志写入类型 
*/

void Log::write_log(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;  //获取当前时间
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
    m_mutex.lock();
    m_count++;//日志行数++
    //如果写入日志时，log文件的创建时间不是当前天 || 当前log文件的行数以及超额
    if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
    { //everyday log 
        char new_log[256] = {0};
        fflush(m_fp);//刷新当前 log 文件流
        fclose(m_fp);//关闭文件流
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, 
                 my_tm.tm_mon + 1, my_tm.tm_mday);
        //写入日志时，log文件的创建时间不是当前天
        if (m_today != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        //当前log文件的行数以及超额
        else{
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, 
                    log_name, m_count / m_split_lines);
        }
        m_fp = fopen(new_log, "a");
    }
    m_mutex.unlock();
    va_list valst;
    va_start(valst, format);
    string log_str;
    m_mutex.lock();
    //写入的具体时间内容格式
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    log_str = m_buf;
    m_mutex.unlock();
    if (m_is_async && !m_log_queue->full())
    {   //异步写入日志
        //将需要写入的内容，入读
        m_log_queue->push(log_str);
    }
    else
    {   //同步写入日志，将写入内容通过文件描述符写入文件
        m_mutex.lock();
        if(fputs(log_str.c_str(), m_fp) < 0) //数据写入文件
        {
            printf("Log写入失败\n");
        }
        m_mutex.unlock();
    }
    va_end(valst);
}
void Log::flush(void)
{
    m_mutex.lock();
    //强制刷新写入流缓冲区
    fflush(m_fp);
    m_mutex.unlock();
}