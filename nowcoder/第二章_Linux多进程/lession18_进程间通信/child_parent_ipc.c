/*
    实现 ps  aux | grep  xxx

    子进程：ps  aux, 子进程将数据发送给 父进程
    父进程：获取到数据，过滤

    pipe
    execlp
    子进程将标准输出 stdout_fileno 重定向到管道的 写端，dup2
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
int main(){
    //1、创建管道
    int  pipefd[2];
    int ret = pipe( pipefd );
    if( -1 == ret ){
        perror("pipe:");
        exit(0);
    }
    //2、创建子进程
    ret = fork();
    if( -1 == ret ){
        perror("fork:");
        exit(0);
    }
    if( ret > 0 ){
        //父进程
        close(pipefd[1]);//关闭写端
        //从管道中读取数据
        char buf[1024] = {0};
        int len = -1;
        while( len = read( pipefd[0] , buf , sizeof(buf) - 1) > 0){
            printf("%s" , buf);//打印到终端
        }
        wait(NULL);//回收子进程
    }
    else if( ret == 0 ){
        //子进程
        close(pipefd[0]);//关闭读端
        //重定向 标准输出 stdout_fileno
        dup2( pipefd[1] , STDOUT_FILENO );
        //重定向后，stdout_fileno指向的为管道的写端
        ret = execlp( "ps" , "ps" , "aux" , NULL);
        if( ret == -1 ){
            perror("execlp:");
        }
        exit(0);
    }
    else{
        perror("fork:");
        exit(0);
    }
    return 0;
}