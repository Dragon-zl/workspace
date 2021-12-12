/*
    案例：
        测试 1s 的时间里，电脑能数多少个数
*/
/*
    实际的时间 = 内核时间 + 用户时间 + 其他消耗的时间
    进行文件 IO 操作的时候，比较浪费时间

    定时器， 与进程的状态无关，
    (自然定时法)
*/
#include <unistd.h>
#include <stdio.h>
int main(){
    int i = 0;
    alarm(1);
    while(1){
        printf("%d\n" , i++);
    }
    return 0;
}