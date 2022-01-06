#include "log.h"


bool Log :: init(const char *file_name, int close_log, 
            int log_buf_size, int split_lines, 
            int max_queue_size){
    //判断 如果设置了 max_queue_size ,则设置为异步
    if(max_queue_size >= 1){
        My_is_async = true;
    }
    
}