#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(){
    //1、创建 socket 套接字
    int fd = socket( AF_INET , SOCK_STREAM , 0);
    if(fd == -1){
        perror("socket:");
        exit(-1);
    }
    //2、连接服务器
    struct sockaddr_in server_addr;
    //协议族
    server_addr.sin_family = AF_INET;
    //设置IP
    inet_pton(AF_INET , "192.168.150.130" , (void *)&server_addr.sin_addr.s_addr); 
    //设置端口
    server_addr.sin_port = htons(9999);//端口号要与服务器的一致
    int ret = connect( fd , (struct sockaddr *)&server_addr , sizeof(server_addr) );
    if(-1 == ret ){
        perror("connect:");
        close(fd);
        exit(-1);
    }
    char recvBuf[1024] = {0};
    int i = 0 ;
    //通讯
    while(i <= 6){
        //发送数据
        sprintf(recvBuf , "data : %d" , i++);
        write( fd , recvBuf , strlen(recvBuf) + 1);

        
        //读数据
        int len = read(fd , recvBuf , sizeof(recvBuf));
        if(-1 == len){      // 调用失败
            perror("read:");
            close(fd);
            exit(-1);
        }
        else if( len > 0){  // 读到数据
            printf("recv server : %s\n" , recvBuf);
        }
        else if(len == 0){  // 服务器断开连接
            printf("server close...\n");
        }
        bzero(recvBuf , sizeof(recvBuf));
        sleep(1);//延时1s
    }
    
    //关闭连接
    close(fd);
    return 0;
}