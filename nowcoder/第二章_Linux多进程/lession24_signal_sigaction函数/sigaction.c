/*
    #include <signal.h>

    int sigaction(int signum, const struct sigaction *act,
                    struct sigaction *oldact);
        功能：检查或者改变信号的处理，信号捕捉
        参数：
            signum：需要捕捉的信号的宏值或编号
            act：捕捉到信号之后的处理动作
            oldact：上一次对信号捕捉设置的动作信息（一般不使用 传递 NULL）
        返回值：
            成功：0
            失败：-1
        struct sigaction {
               void     (*sa_handler)(int);  //函数指针，信号捕捉到的处理函数
               void     (*sa_sigaction)(int, siginfo_t *, void *); //不常用
               sigset_t   sa_mask;//临时阻塞信号集，在信号捕捉函数执行过程中，临时阻塞某些信号
               int        sa_flags;//设置使用哪个函数来对捕捉到的信号进行处理
               //这个值可以是 0 ，表示使用 sa_handler。也可以是 SA_SIGINFO ,表示使用 sa_sigaction 函数来处理
               void     (*sa_restorer)(void);//被废弃掉了，填 NULL 即可
           };
        
        sigaction 比 signal 函数更加适应各种不同版本的Linux系统，
        建议使用 sigaction 函数 而非 signal
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
    struct sigaction act;
    act.sa_flags = 0;//设置使用 sa_handler 处理函数
    act.sa_handler = My_alarm;//设置处理函数
    sigemptyset( &act.sa_mask);//清空临时阻塞信号集，表示不阻塞任何信号

    int sig_ret = sigaction( SIGALRM , &act, NULL );
    if( sig_ret == -1){
         
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