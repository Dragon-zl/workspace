/*
    案例：
        创建一个守护进程，每隔 2s 获取系统时间，并写入到磁盘文件中
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <string.h>
void my_work(int num){
    //获取 当前时间（以计算机元年 1970为参考)
    time_t timer = time( NULL );
    //将时间转化为 相应参数
    struct tm * my_tm = localtime( &timer );
    //char buf[512] = {0};
    //sprintf( buf , "%d-%d-%d  %d:%d:%d" , my_tm->tm_year , my_tm->tm_mon ,
    //            my_tm->tm_mday , my_tm->tm_hour , my_tm->tm_min , my_tm->tm_sec );
    //printf("%s" , buf);
    char * str = asctime( my_tm );
    int fd = open( "time.txt" , O_CREAT | O_RDWR | O_APPEND, 0664);
    write( fd , str , strlen(str));
    close(fd);
}
int main(){
    //1、fork 一个子进程 ， 杀死父进程
    pid_t  pid = fork();
    if( pid > 0 ){
        exit(0);
    }
    //2、子进程创建一个新会话
    setsid();
    //3、设置权限
    umask(022);
    //4、修改工作目录
    chdir("/home/dzl/");
    //5、关闭守护进程从其父进程继承而来的所有打开着的文件描述符、重定向 
    int fd = open("/dev/null" , O_RDWR);
    dup2( fd , STDIN_FILENO);
    dup2( fd , STDOUT_FILENO);
    dup2( fd , STDERR_FILENO);
    close(fd);
    //6、核心业务逻辑
    //设置信号捕捉
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = my_work;
    sigemptyset( &act.sa_mask );
    sigaction( SIGALRM, &act, NULL);
    //创建定时器
    struct itimerval  val;
    val.it_interval.tv_sec = 2;
    val.it_interval.tv_usec = 0;
    val.it_value.tv_sec = 2;
    val.it_value.tv_usec = 0;
    setitimer( ITIMER_REAL , &val , NULL);

    while(1);
    return 0;
}