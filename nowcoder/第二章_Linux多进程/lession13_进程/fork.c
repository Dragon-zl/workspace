#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
/*
    pid_t fork(void);
        作用：创建一个子进程
        返回值：
            fork()的返回值会返回两次，一次是在父进程中，一次是在子进程中。
            在父进程中返回子进程的pid
            在子进程中返回 0
            区分父进程和子进程：通过fork的返回值

            失败：创建子进程失败返回 -1 ， 并设置errno 
    
*/

int main()
{
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