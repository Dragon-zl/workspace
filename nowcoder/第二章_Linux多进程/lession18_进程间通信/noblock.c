/*
    设置管道非阻塞：
        使用 fcontl 设置文件描述符为非阻塞

            // 获取文件描述符状态flag
            int flag = fcntl( fd , F_GETFL);
            // 修改文件描述符状态的flag ， 给flag加入 O_NONBLOCK为非阻塞
            flag |= O_NONBLOCK; 
            int ret = fcntl( fd , F_SETFL , flag);
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
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

        // 获取文件描述符状态flag
        int flag = fcntl( pipefd[0] , F_GETFL);
        // 修改文件描述符状态的flag ， 给flag加入 O_NONBLOCK为非阻塞
        flag |= O_NONBLOCK; 
        int ret = fcntl( pipefd[0] , F_SETFL , flag);

        for(  ;   ;  ){
            int len = read( pipefd[0] , buf , sizeof(buf));
            printf("len : %d\n" , len);
            printf("parent recv: %s , pid = %d\n" , buf , getpid());
            bzero(buf , 1024);
            sleep(1);
        }
        
    }
    else if( pid == 0 ){
        //子进程
        char buf[1024] = {0};
        for(int i = 0 ; i < 3 ; i++ ){
            char * str = "hello i am child process";
            write( pipefd[1] , str , strlen(str));
            sleep(2);
        }
        
    }
    return 0;
}