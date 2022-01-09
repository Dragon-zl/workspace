#include "log.h"
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string>
int Log :: My_close_log = 1;
void * flush_log_thread(void * arg){

}
bool Log :: init(const char *file_name, int close_log, 
            int log_buf_size, int split_lines, 
            int max_queue_size){
    //判断 如果设置了 max_queue_size ,则设置为异步
    if(max_queue_size >= 1){
        My_is_async = true;
        My_log_queue = new blockqueue<string>(max_queue_size);
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

    const  char *p = strstr(file_name , '/');
    char log_full_name[256] = {0};

    if( p == NULL){
        //格式化字符串：文件名
        snprintf( log_full_name , 255 , "%d_%02d_%02d_%s" , My_tm.tm_year + 1900 ,
                    My_tm.tm_mon + 1 , My_tm.tm_mday , file_name);
        
    }
}
