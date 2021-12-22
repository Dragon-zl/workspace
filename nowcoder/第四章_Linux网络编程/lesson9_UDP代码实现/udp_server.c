#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(){
    //1、创建一个 通信的 socket
    int fd = socket(AF_INET , SOCK_DGRAM  , 0);
    if(-1 == fd){
        perror("socket");
        exit(-1);
    }
    struct  sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    //2、绑定
    int ret = bind(fd , (struct sockaddr *)&addr , sizeof(addr));
    if( -1 == ret ){
        perror("bind");
        exit(-1);
    }

    //3、通信
    while(1){
        char buf[128] = {0};
        struct  sockaddr_in client_addr;
        socklen_t  size = sizeof(client_addr);
        //接收数据
        int len = recvfrom( fd , buf , sizeof(buf) , 0 , (struct sockaddr *)&client_addr , &size);
        if( len == -1){
            perror("recvfrom");
            exit(-1);
        }
        //打印另一端的信息
        char ip_buf[16] = {0};
        inet_ntop( AF_INET , &client_addr.sin_addr.s_addr , ip_buf , sizeof(ip_buf));
        printf("client ip %s , port %d\n" , ip_buf , ntohs( client_addr.sin_port ));

        printf("client data : %s\n" , buf);

        //发送数据
        sendto( fd , buf , strlen(buf) + 1 , 0 , (struct sockaddr *)&client_addr , sizeof(client_addr));
    }
    close(fd);
    return 0;
}