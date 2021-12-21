#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>
/*
    服务端
*/
int main(){
    //1、创建套接字
    int lfd = socket( AF_INET, SOCK_STREAM  , 0);
    if( lfd == -1){
        perror("socket:");
        exit(-1);
    }
    //2、绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9999);
    int ret = bind( lfd,  (struct sockaddr *)&addr, sizeof(addr));
    if( ret == -1){
        perror("bind:");
        exit(-1);
    }
    //3、监听
    ret =  listen( lfd, 8);
    if( ret == -1){
        perror("listen:");
        exit(-1);
    }
    //初始化监测的文件描述符数组
    struct  pollfd  fds[1024];
    for( int i = 0 ; i < 1024 ; i++){
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = lfd;//设置监听文件描述符，作为第一个元素
    int nfds = 0;
    //4、等待客户端连接
    while(1){

        //调用 poll 系统函数，让内核帮监测哪些文件描述符有数据
        ret = poll( fds , nfds + 1 , -1);//永久阻塞
        if(-1 == ret ){
            perror("poll");
            exit(-1);
        }
        else if( 0 == ret){
            continue;
        }
        else if( ret > 0){
            //说明监测到了文件描述符对应的缓冲区的数据发生了改变
            if( fds[0].revents & POLLIN ){   //这里要用位运算
                //表示有新的客户端连接进来
                struct sockaddr_in client_addr;
                socklen_t size = sizeof(client_addr);
                int client_fd = accept( lfd , (struct sockaddr *)&client_addr , &size);
                //打印客户端的信息
                char  client_ip[16] = {0};
                inet_ntop( AF_INET , &client_addr.sin_addr.s_addr , client_ip , sizeof(client_ip));
                unsigned  short clientport = ntohs(client_addr.sin_port);
                printf("client ip is %s , client port is %d\n" , client_ip, clientport);
                //将新的文件描述符加入到集合中
                int i;
                for(i = 1 ; i < 1024 ; i++){
                    if(fds[i].fd == -1){
                        fds[i].fd = client_fd;
                        fds[i].events = POLLIN;
                        break;
                    }
                }
                //更新索引
                nfds = nfds > client_fd ? nfds : client_fd;
            }
            //遍历文件描述符
            for(int i =  1 ; i< nfds ; i++){
                if( fds[i].revents & POLLIN ){
                    //说明这个文件描述符的客户端发送了数据
                    char buf[1024] = {0};
                    int len = read(fds[i].fd , buf , sizeof(buf));
                    if(len == -1){
                        perror("read:");
                        exit(-1);
                    }
                    else if(len == 0){//客户端断开
                        printf("client close\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        fds[i].events = POLLIN;
                    }
                    else if(len > 0){
                        printf("client data: %s\n" , buf);
                        write( fds[i].fd  , buf , strlen(buf) + 1);
                    }
                }
            }
        }
    }
    close(lfd);
    return 0;
}