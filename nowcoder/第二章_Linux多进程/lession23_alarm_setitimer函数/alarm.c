
/*
    #include <unistd.h>
    unsigned int alarm(unsigned int seconds);
        功能：
            设置定时器，
            函数倒计时 seconds 后，
            给当前进程发送一个信号：
                SIGALARM
        参数：
            seconds
            单位：秒
            如果参数为 0 ，定时器无效，
            取消定时器：通过alarm(0)
        返回值：
            -之前没有设置定时器  返回 0
            -之前有设置定时器，返回之前的定时器 剩余的时间

    -SIGALARM：
        默认终止当前进程，每一个进程都有且只有唯一的一个定时器

    alarm 是不阻塞的
*/
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
int main(){

    int seconds = alarm(10);//定时 5s
    printf("seconds = %d\n" ,seconds );

    sleep(2);
    seconds = alarm(3);
    printf("seconds = %d\n" ,seconds );

    while(1){   //死循环

    }
    return 0;
}