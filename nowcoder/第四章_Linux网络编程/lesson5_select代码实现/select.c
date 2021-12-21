#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
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
    //4、等待客户端连接
    fd_set  rdset , tmp;
    FD_ZERO(&rdset);
    FD_SET( lfd, &rdset);
    int maxfd = lfd;
    while(1){
        tmp = rdset;
        //调用select系统函数，让内核帮监测哪些文件描述符有数据
        ret = select( maxfd + 1 , &tmp , NULL  , NULL , NULL);//永久阻塞
        if(-1 == ret ){
            perror("select");
            exit(-1);
        }
        else if( 0 == ret){
            continue;
        }
        else if( ret > 0){
            //说明监测到了文件描述符对应的缓冲区的数据发生了改变
            if( FD_ISSET( lfd , &tmp) ){
                //表示有新的客户端连接进来
                struct sockaddr_in client_addr;
                socklen_t size = sizeof(client_addr);
                int client_fd = accept( lfd , (struct sockaddr *)&client_addr , &size);
                //将新的文件描述符加入到集合中
                FD_SET(client_fd , &rdset);
                //判断是否更新maxfd 
                maxfd = maxfd > client_fd ? maxfd:client_fd;
            }
            //遍历文件描述符
            for(int i = lfd + 1 ; i< maxfd + 1; i++){
                if( FD_ISSET(i , &tmp) ){
                    //说明这个文件描述符的客户端发送了数据
                    char buf[1024] = {0};
                    int len = read(i , buf , sizeof(buf));
                    if(len == -1){
                        perror("read:");
                        exit(-1);
                    }
                    else if(len == 0){
                        printf("client close\n");
                        close(i);
                        FD_CLR( i ,&rdset);
                    }
                    else if(len > 0){
                        printf("client data: %s\n" , buf);
                        write( i , buf , strlen(buf) + 1);
                    }
                }
            }
        }
    }
    close(lfd);
    return 0;
}