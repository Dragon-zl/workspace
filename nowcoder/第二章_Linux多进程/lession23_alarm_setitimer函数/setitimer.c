/*
#include <sys/time.h>

    int setitimer(int which, const struct itimerval *new_value,
                    struct itimerval *old_value);
        功能：设置定时器，可以替代 alarm 函数
              精确度高(us)，可以实现周期性定时
        参数：
            int which  （指定定时器以什么时间 计时）
                ITIMER_REAL：真实时间， 时间到达 发送 SIGALRM  (常用)
                ITIMER_VIRTUAL：用户时间  时间到达 ， 发送 SIGVTALRM
                ITIMER_PROF：内核+用户时间  发送 SIGPROF
            new_value: (设置定时器的属性)
                    struct itimerval {      //定时器结构体
                        struct timeval it_interval;//间隔时间
                        struct timeval it_value;//延迟多长时间，执行定时器    
                            };

                    struct timeval {        //时间结构体
                        time_t      tv_sec;//秒      
                        suseconds_t tv_usec;//微妙   
                    };
                举例：10s后，开始定时，每隔2s定时一次
            old_value：(记录上一次定时的时间参数 ， 一般不使用，指定 NULL)
        返回值：
            成功：返回0
            失败：返回-1
        
        setitimer 函数是非阻塞的
        setitimer 函数在设置的延时时间完成后，就立即可定时响应一次
*/
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

/*
    完成功能：
        3s后，每隔2s定时一次
*/
int main(){
    struct itimerval new_value;
    //设置参数
    //设置间隔时间为 2 s
    new_value.it_interval.tv_sec = 2;   
    new_value.it_interval.tv_usec = 0;

    //设置延迟的时间
    new_value.it_value.tv_sec = 3;  
    new_value.it_value.tv_usec = 0;
    int  ret = setitimer(ITIMER_REAL ,  &new_value , NULL);
    if( -1 == ret){
        perror("setitimer:");
        exit(0);
    }
    printf("定时器开始\n");

    while(1){

    }
    return 0;
}