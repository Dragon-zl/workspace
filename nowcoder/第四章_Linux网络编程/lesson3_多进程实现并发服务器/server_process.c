#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
int main(){
    //1、创建socket
    int lfd = socket( AF_INET , SOCK_STREAM , 0);
    if( -1 == lfd ){
        perror("socket:");
        exit(-1);
    }
    //2、绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind( lfd , (struct sockaddr *)&saddr , sizeof(saddr));
    if( -1 == ret ){
        perror("bind:");
        close(lfd);
        exit(-1);
    }
    //3、监听
    ret = listen( lfd , 8 );
    if( -1 == ret ){
        perror("listen:");
        close(lfd);
        exit(-1);
    }
    //4、循环等待客户端连接
    while(1){
        //接收连接
        struct sockaddr_in client_addr;
        socklen_t size = sizeof(client_addr);
        int client_fd = accept( lfd ,(struct sockaddr *)&client_addr , &size);
        if( -1 == client_fd ){
            perror("accept:");
            close(lfd);
            exit(-1);
        }
        //创建子进程
        pid_t  pid = fork();
        if( pid == 0 ){
            //子进程
            //获取客户端信息
            //输出客户端信息
            char  client_ip[16] = {0};
            inet_ntop( AF_INET , &client_addr.sin_addr.s_addr , client_ip , sizeof(client_ip));
            unsigned  short clientport = ntohs(client_addr.sin_port);
            printf("client ip is %s , client port is %d\n" , client_ip, clientport);

            //开始通信
            //获取客户端的数据
            char recvBuf[1024] = {0};
            while(1){
                int len = read( client_fd , recvBuf , sizeof(recvBuf));
                if(-1 == len){      // 调用失败
                    perror("read:");
                    close(client_fd);
                    close(lfd);
                    exit(-1);
                }
                else if( len > 0){  // 读到数据
                    printf("recv client data : %s\n" , recvBuf);
                }
                else if(len == 0){  // 客户端断开连接
                    printf("client close...\n");
                    break;
                }
                //回射
                write( client_fd , recvBuf , strlen(recvBuf));
            }
            //关闭连接
            close(client_fd);
            exit(0);//退出子进程
        }
    }
    close(lfd);//关闭监听文件描述符
}