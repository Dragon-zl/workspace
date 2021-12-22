
/*
    服务端
*/
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main(){
    //0、删除通信文件,防止出现重复绑定
    unlink("server.sock");
    //1、创建一个 监听的套接字
    int lfd = socket( AF_LOCAL , SOCK_STREAM , 0);
    if(-1 == lfd){
        perror("socket");
        exit(-1);
    }
    //2、绑定套接字
    struct  sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path , "server.sock");

    int ret = bind( lfd , (struct sockaddr *)&addr ,  sizeof(addr));
    if( -1 == ret ){
        perror("bind");
        exit(-1);
    }
    //3、监听
    ret = listen(lfd , 100);
    if( -1 == ret ){
        perror("listen");
        exit(-1);
    }
    //4、等待并接收连接请求
    struct  sockaddr_un client_addr;
    socklen_t size = sizeof(client_addr);
	int client_fd = accept( lfd , (struct sockaddr *)&client_addr , &size);
    if( client_fd == -1){
        perror("accept");
        exit(-1);
    }
    printf("client socket filename: %s\n" , client_addr.sun_path);

    //5、通信
    while(1){
        char buf[128] = {0};
        int len = recv( client_fd , buf , sizeof(buf) , 0);
        if( -1 == len ){
            perror("recv");
            exit(-1);
        }
        else if( 0 == len ){
            printf("client close\n");
            break;
        }
        else if( len > 0){
            printf("client data : %s\n" , buf);
            send( client_fd , buf , len , 0);
        }
    }
    close(client_fd);
    close(lfd);
}