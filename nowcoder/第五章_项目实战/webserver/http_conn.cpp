#include "http_conn.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int http_conn::My_epollfd = -1;     
int http_conn::My_users_count = 0;    

//设置文件描述符非阻塞
void  setnonblocking(int fd){
    int old_flag = fcntl( fd , F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl( fd , F_SETFL , new_flag);
}
//添加文件描述符到 epoll 实例中
void  addfd_epoll(int epoll , int fd , bool one_shot){
    epoll_event  event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP ; //EPOLLRDHUP ：监听该事件可以使得当 客户端与服务端异常断开时，
                                            //内核能够检测得到，并自动处理，无需应用层去判断处理
    //防止 同一个 socket 触发两个线程去处理
    if( one_shot ){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epoll , EPOLL_CTL_ADD ,fd , &event);
    //设置文件描述符非阻塞
    setnonblocking(fd);
}
//从epoll实例中删除文件描述符
void  removefd_epoll(int epoll , int fd){
    epoll_ctl( epoll , EPOLL_CTL_DEL , fd , 0);
    close(fd);
}
//修改 epoll 实例中的文件描述符 , 每次修改都要重置 EPOLLONESHOT 事件
void  modfd_epoll(int epoll , int fd , int ev){
    epoll_event  event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT;
    epoll_ctl( epoll , EPOLL_CTL_MOD , fd , &event);
}


http_conn::http_conn(/* args */)
{
}

http_conn::~http_conn()
{
}
//初始化新接收的客户端
void http_conn::init(int connect_fd , struct sockaddr_in & client_addr){
    My_sockfd = connect_fd;
    My_address = client_addr;

    //设置端口复用
    int optname = 1;
    setsockopt( My_sockfd , SOL_SOCKET , SO_REUSEADDR , &optname , sizeof(optname));

    //添加到 epoll 实例当中
    addfd_epoll( My_epollfd , My_sockfd , true);
    My_users_count++;   //总的连接用户数 + 1
}
//非阻塞的读
bool http_conn:: read(){
    printf("一次性读出数据\n");
    return true;
}
//非阻塞的写
bool http_conn:: write(){
    printf("一次性写入数据\n");
    return true;
}
//由线程池中的工作线程调用，处理客户端请求的入口函数
void http_conn:: process(){
    //解析 HTTP 请求
    printf("parse  request, create  response\n");
    //生成响应
}
//关闭连接
void http_conn:: close_connect(){
    //将套接字从 epoll实例中哦移除
    removefd_epoll( My_epollfd , My_sockfd );
    My_sockfd = -1;
    //连接用户数 减 1
    My_users_count--;
}
