/*
    #include <signal.h>

    typedef void (*sighandler_t)(int);
    sighandler_t signal(int signum, sighandler_t handler);
        功能：
            设置某个信号的捕捉行为
        参数：
            signum：要捕捉的信号
            handler：
                SIG_IGN：忽略信号
                SIG_DFL：使用默认的信号行为
                回调函数：由内核调用该函数，程序员只负责编写。捕捉到信号后去使用该函数去处理
        返回值：
            成功：返回上一次注册的信号处理函数的地址。第一次调用 返回NULL
            失败：返回 SIG_ERR 宏

    注意：
    The signals SIGKILL and SIGSTOP cannot be caught or ignored.
*/

/*
    完成功能：
        3s后，每隔2s定时一次
        使用 signal 函数去修改信号的处理
*/
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
typedef void (*sighandler_t)(int);

void My_alarm(int num){
    printf("捕捉到信号 ， 信号编号 = %d\n" , num);
}
int main(){
    //注册信号捕捉
    //signal( SIGALRM , SIG_IGN);//忽略信号
    //signal( SIGALRM , SIG_DFL);//信号默认的处理
    sighandler_t  sig_ret = signal( SIGALRM , My_alarm );
    if( sig_ret == SIG_ERR){
        perror("signal:");
        exit(0);
    }

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