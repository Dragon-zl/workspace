#include <stdio.h>

/*
    #include <sys/types.h>
    #include <sys/wait.h>
    pid_t wait(int *wstatus);
        功能：等待任意一个子进程结束，如果任意一个子进程结束，此函数会回收子进程
        参数：
            进程退出时的状态信息，int 类型的地址，传出参数
        返回值：
            - 成功：返回子进程的 ID
            - 失败：返回 -1 (所以子进程结束 || 调用函数失败)
    
    调用 wait() 函数，进程会被挂起(阻塞)。
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    //有一个进程 , 创建 5 个子进程(兄弟)
    pid_t pid;
    for( int i = 0 ; i < 5 ; i++ )
    {
        pid = fork();
        if( pid == 0 ){
            break;
        }
    }
    if( pid > 0){
        while(1){
            printf("parent , pid = %d\n" , getpid());
            int st;
            int ret = wait( &st );//如果参数 填 NULL， 即不需要保存子进程结束的状态信息
            if( -1 == ret ){
                break;  //所以子进程都结束了
            }
            if( WIFEXITED(st) ){
                printf("子进程结束的状态吗 = %d\n" , WEXITSTATUS(st) );
            }
            if( WIFSIGNALED(st) ){
                printf("子进程被干掉的信号 = %d\n" , WTERMSIG(st) );
            }
            
            printf("process die , pid = %d\n" , ret);
        }
    }
    else if( pid == 0 ){
        printf("child , pid = %d\n" , getpid());
        sleep(1);
        exit(0);//正常退出
    }
    return 0;
}