#ifndef LST_TIMER_H
#define LST_TIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>
#include "../log/log.h"
#include "../tcpclient/tcpclient.h"
class util_timer;
//该结构体保存客户端信息,连接资源包括客户端套接字地址、文件描述符和定时器
struct client_data
{
    sockaddr_in address; //保存客户端ip地址
    int sockfd;          //TCP连接套接字
    util_timer *timer;   //定时器指针
};
//定时器类
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    //超时时间
    time_t expire; 
    //回调函数
    void (*cb_func)(client_data *);
    //客户端连接资源
    client_data *user_data;
    //前向定时器
    util_timer *prev;
    //后继定时器
    util_timer *next;
};
//定时器容器类: 带头尾结点的升序双向链表
class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();
    //将目标定时器添加到链表中，添加时按照升序添加
    void add_timer(util_timer *timer);    
    //当定时任务发生变化,调整对应定时器在链表中的位置
    void adjust_timer(util_timer *timer); 
    void del_timer(util_timer *timer);    //从链表中，清空一个成员
    void tick();                          //超时处理函数
private:
    //私有成员，被公有成员add_timer和adjust_time调用
    //主要用于调整链表内部结点
    void add_timer(util_timer *timer, util_timer *lst_head);
    util_timer *head; //头指针
    util_timer *tail; //尾指针
};
//注册定时信号及处理函数
class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;//管道文件描述符指针，用于定时器事件通知主循环
    sort_timer_lst m_timer_lst;//定时器链表
    static int u_epollfd;
    int m_TIMESLOT;//定时器时间
};
//定时器回调函数
void cb_func(client_data *user_data);
#endif