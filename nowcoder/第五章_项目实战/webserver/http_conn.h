#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>
class http_conn
{
private:
    int  My_sockfd;             //该 HTTP 连接的socket
    sockaddr_in  My_address;    //通信的socket地址
public:
    static int My_epollfd;      //所有的 socket 事件，都共享一个 epoll 实例
    static int My_users_count;    //正在连接的用户数量


    http_conn(/* args */);
    ~http_conn();

    //初始化新接收的客户端
    void init(int connect_fd , struct sockaddr_in & client_addr);
    void process();//处理客户端请求
};







#endif