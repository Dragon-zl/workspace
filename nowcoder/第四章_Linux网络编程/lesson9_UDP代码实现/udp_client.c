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
    //服务器的地址信息
    struct  sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    inet_pton( AF_INET , "192.168.150.130" , &server_addr.sin_addr.s_addr);
    
    //2、通信
    int num = 0; 
    while(1){
        char send_buf[128] = {0};
        sprintf(send_buf , "hello , I am client %d" , num++);
        //发送数据
        sendto( fd , send_buf , sizeof(send_buf) , 0 , (struct sockaddr *)&server_addr , sizeof(server_addr));

        bzero(send_buf , 0);
        //接收数据
        int len = recvfrom( fd , send_buf , sizeof(send_buf) , 0 , NULL , NULL);
        if( len == -1){
            perror("recvfrom");
            exit(-1);
        }
        //打印另一端的信息
        printf("server data : %s\n" , send_buf);
        sleep(1);
        
    }
    close(fd);
    return 0;
}