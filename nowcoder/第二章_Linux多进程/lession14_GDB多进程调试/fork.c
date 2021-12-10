#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>



int main()
{
    printf("begin\n");

    pid_t pid = fork();
    //创建子进程

    if( pid > 0){
        //当前是父进程
        printf("pid : %d \n" , pid);
        //打印子进程的 pid
        printf("I am parent process , pid : %d , ppid: %d\n" , getpid() , getppid);
    }
    else if( pid == 0 ){
        //当前是子进程
        printf("I am child process , pid : %d , ppid: %d\n" , getpid() , getppid);
    }

    //for循环，这个for循环是子进程和父进程共享的代码
    for(int i = 0 ; i < 3 ; i++)
    {
        printf("i : %d , pid : %d \n" , i , getpid());
        sleep(1);
    }
    return 0;
}