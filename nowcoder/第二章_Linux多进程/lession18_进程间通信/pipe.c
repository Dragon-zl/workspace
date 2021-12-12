/*
    #include <unistd.h>
    int pipe(int pipefd[2]);
        功能：创建一个匿名管道
        参数：
            传出参数
            pipefd[0]  对应管道的读端
            pipefd[1]  对应管道的写端
        返回值：
            成功：返回 0
            失败：返回 -1
    管道 默认是阻塞的：
        如果管道中没有数据，read会阻塞
        如果管道满了 ， write会阻塞
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//子进程发送数据给 父进程 ， 父进程收到 输出
int main(){
    //1、在fork 之前创建管道
    int  pipefd[2];
    int ret = pipe( pipefd );
    if( -1 == ret ){
        perror("pipe:");
        exit(0);
    }
    //2、创建 子进程
    pid_t  pid = fork();
    if( pid > 0 ){
        //父进程
        //从管道中 读取数据
        char buf[1024] = {0};
        for(int i = 0 ; i < 3 ; i++){
            int len = read( pipefd[0] , buf , sizeof(buf));
            printf("parent recv: %s , pid = %d\n" , buf , getpid());
            char * str = "hello i am parent process";
            write( pipefd[1] , str , strlen(str));
            sleep(1);
        }
        
    }
    else if( pid == 0 ){
        //子进程
        char buf[1024] = {0};
        for(int i = 0 ; i < 3 ; i++ ){
            char * str = "hello i am child process";
            write( pipefd[1] , str , strlen(str));
            sleep(1);
            int len = read( pipefd[0] , buf , sizeof(buf));
            printf("parent recv: %s , pid = %d\n" , buf , getpid());
        }
        
    }
    return 0;
}