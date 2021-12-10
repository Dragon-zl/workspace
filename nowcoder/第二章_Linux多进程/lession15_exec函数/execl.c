#include <stdio.h>
#include <unistd.h>
/*
    int execl(const char *path, const char *arg, ...);
        参数：
            - path ：需要指定的执行的文件的路径或者名称
            /home/dzl/workspace/lession_exec函数/a.out  或者 a.out 推荐使用绝对路径
            - arg ：是可执行文件所需要的参数列表
                第一个参数一般是可执行程序的文件名，没什么用
                从第二个参数开始往后，就是程序执行所需要的参数列表。
                参数最后需要以 NULL 结束(哨兵)
            -返回值
                只有调用失败，才有返回值，返回-1，并且设置errno
*/
int main()
{
    //在子进程中调用 execl 函数
    pid_t  pid = fork();

    if( pid > 0){
        //父进程
        printf("I am parent process , pid = %d\n" , getpid());
        sleep(1);
    }
    else{
        //子进程
        execl("hello" , "hello" , NULL);
        printf("I am child process , pid = %d\n" , getpid());
    }
    for(int i = 0 ; i < 3 ; i++){
        printf("i = %d , pid = %d\n" , i , getpid());
    }
    return 0;
}