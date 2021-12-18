#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

struct sockinfo{
    int fd ; //文件描述符
    struct sockaddr_in addr;
    pthread_t  tid;//线程tid
};
struct sockinfo  client_info[128];
void * My_work(void * arg){
    struct sockinfo * pinfo = (struct sockinfo *)arg;
    //输出客户端信息
    char  client_ip[16] = {0};
    inet_ntop( AF_INET , &pinfo->addr.sin_addr.s_addr , client_ip , sizeof(client_ip));
    unsigned  short clientport = ntohs(pinfo->addr.sin_port);
    printf("client ip is %s , client port is %d\n" , client_ip, clientport);

    //开始通信
    //获取客户端的数据
    char recvBuf[1024] = {0};
    while(1){
        int len = read( pinfo->fd , recvBuf , sizeof(recvBuf));
        if(-1 == len){      // 调用失败
            perror("read:");
            close(pinfo->fd);
            exit(-1);
        }
        else if( len > 0){  // 读到数据
            printf("recv client : %s\n" , recvBuf);
        }
        else if(len == 0){  // 客户端断开连接
            printf("client close...\n");
            break;
        }
        //回射
        write( pinfo->fd , recvBuf , strlen(recvBuf) + 1);
    }
    //关闭连接
    close(pinfo->fd);
    exit(0);    // 退出当前子进程
}
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
    //初始化客户端连接信息数组
    int max = sizeof(client_info) / sizeof(client_info[0]);
    for( int i = 0 ; i < max ; i++){
        bzero( &client_info[i]  ,  sizeof(client_info[i]));
        client_info[i].fd = -1;
        client_info[i].tid = -1;
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
        //创建子线程
        struct sockinfo * pinfo;
        for(int i = 0 ; i< max ; i++){
            //找到一个可以使用的信息
            if( client_info[i].fd == -1){
                pinfo = &client_info[i];
                break;
            }
            if( i == max -1 )//没有可用的线程信息
            {
                sleep(1);
                i = 0;
            }
        }
        pinfo->fd = client_fd;
        memcpy( &pinfo->addr , &client_addr , size);

        pthread_create( &pinfo->tid , NULL , My_work , (void *)pinfo);
        //设置线程分离
        pthread_detach(pinfo->fd);
    }
    close(lfd);//关闭监听文件描述符
    return 0;
}