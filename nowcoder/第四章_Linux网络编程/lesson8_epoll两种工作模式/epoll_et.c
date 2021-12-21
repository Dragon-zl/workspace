#include <errno.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
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
    //创建一个 epoll 实例
    int epfd = epoll_create( 100 );
    if( -1 == epfd ){
        perror("epoll_create");
        exit(-1);
    }
    //对epoll实例进行操作: 增、删、改
    struct epoll_event event;
    event.events = EPOLLIN;//设置监听事件
    event.data.fd = lfd;
    ret = epoll_ctl( epfd, EPOLL_CTL_ADD , lfd, &event);

    struct epoll_event events[1024];
    while(1){
        /*监测函数*/
        ret  = epoll_wait( epfd, events,  sizeof(events) , -1 );
        if( -1 == ret ){
            perror("epoll_wait");
            exit(-1);
        }
        printf("ret = %d\n" , ret);

        //遍历数组
        for(int i = 0 ; i < ret ; i++){
            //定义临时变量，当前遍历到的文件描述符
            int curfd = events[i].data.fd;
            if(curfd == lfd){
                //监听的文件描述符有客户端连接
                struct sockaddr_in client_addr;
                socklen_t size = sizeof(client_addr);
                int client_fd = accept( lfd , (struct sockaddr *)&client_addr , &size);
                //设置 client_fd 为非阻塞
                int flag = fcntl(client_fd , F_GETFD);//先获取
                flag |= O_NONBLOCK;
                fcntl(client_fd , F_SETFD  , flag);//再设置

                //打印客户端的信息
                char  client_ip[16] = {0};
                inet_ntop( AF_INET , &client_addr.sin_addr.s_addr , client_ip , sizeof(client_ip));
                unsigned  short clientport = ntohs(client_addr.sin_port);
                printf("client ip is %s , client port is %d\n" , client_ip, clientport);

                //将新的与客户端通信的文件描述符，添加到epfd中
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;//设置监听读事件 边沿触发
                event.data.fd = client_fd;
                epoll_ctl( epfd, EPOLL_CTL_ADD , client_fd, &event);
            }
            else{
                //有数据到达，需要通信
                //读取所有数据(循环)，且设置 client_fd 为非阻塞
                char buf[5] = {0};
                int len = 0;
                while( (len = read(curfd , buf , sizeof(buf))) > 0){
                    //打印数据
                    printf("recv data :%s\n" , buf);
                    write( curfd , buf  , strlen(buf));
                }
                if(len == 0){
                    //客户端断开连接
                    printf("client close....\n");
                }
                else if(len == -1){
                    if(errno == EAGAIN){
                        printf("data over...\n");
                    }
                    else{
                        perror("read");
                        exit(-1);  
                    }
                }
            }
        }
    }
    close(lfd);
    close(epfd);//关闭epoll实例
    return 0;
}