
/*
    客户端
*/
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main(){
    //0、删除通信文件  防止出现重复绑定
    unlink("client.sock");
    //1、创建一个 套接字
    int fd = socket( AF_LOCAL , SOCK_STREAM , 0);
    if(-1 == fd){
        perror("socket");
        exit(-1);
    }
    //2、绑定套接字
    struct  sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path , "client.sock");

    int ret = bind( fd , (struct sockaddr *)&addr ,  sizeof(addr));
    if( -1 == ret ){
        perror("bind");
        exit(-1);
    }
    
    //3、连接服务器
    struct  sockaddr_un server_addr;
    server_addr.sun_family = AF_LOCAL;
    strcpy( server_addr.sun_path  ,  "server.sock");
    ret = connect( fd ,  (struct sockaddr *)&server_addr , sizeof(server_addr) );
    if( -1 == ret ){
        perror("connect");
        exit(-1);
    }
    //5、通信
    int num = 0;
    while(1){
        char buf[128] = {0};
        sprintf(buf , "hello ,I am client %d" , num++);
        send( fd , buf , strlen(buf) + 1 , 0);

        bzero( buf , 0);
        int len = recv( fd , buf , sizeof(buf) , 0);
        if( -1 == len ){
            perror("recv");
            exit(-1);
        }
        else if( 0 == len ){
            printf("server close\n");
            break;
        }
        else if( len > 0){
            printf("server data : %s\n" , buf);
        }
        sleep(1);
    }
    close(fd);
}