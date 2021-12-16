/*
    TCP服务端
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
int main(){

    //1、创建 socket(用于监听的套接字)
    int lfd = socket( AF_INET , SOCK_STREAM , 0);
    if( -1 == lfd ){
        perror("socket:");
        exit(-1);
    }

    //2、绑定
    struct sockaddr_in addr;
    //协议族
    addr.sin_family = AF_INET;
    //设置IP
    //inet_pton(AF_INET , "192.168.1.1" , (void *)addr.sin_addr.s_addr); 
    addr.sin_addr.s_addr = INADDR_ANY;//也可以 不用宏直接用 0 ，表示使用任意网卡的地址
    //设置端口
    addr.sin_port = htons(9999);
    int ret = bind( lfd , (struct sockaddr *)&addr, sizeof(addr));
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

    //4、接收客户端的连接
    struct sockaddr_in client_addr;//用于保存客户端信息
    socklen_t size = sizeof(client_addr);
    int client_fd = accept( lfd , (struct sockaddr *)&client_addr , &size);
    if( -1 == client_fd ){
        perror("accept:");
        close(lfd);
        exit(-1);
    }
    //输出客户端信息
    char  client_ip[16] = {0};
    //网络字节序 ip  -》 主机字节序 ip
    inet_ntop( AF_INET , &client_addr.sin_addr.s_addr , client_ip , sizeof(client_ip)); 
    //网络字节序 port -》 主机字节序 port
    unsigned  short  clientport = ntohs(client_addr.sin_port);

    printf("client ip is %s , client port is %d\n" , client_ip, clientport);

    //5、通信
    //获取客户端的数据
    char recvBuf[1024] = {0};
    int len = read( client_fd ,  recvBuf , sizeof(recvBuf));
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
    }
    //给客户端发送数据
    char * data = "hello , I am server";
    write( client_fd , data , strlen(data));
    //关闭文件描述符
    close(client_fd);
    close(lfd);
    return 0;
}