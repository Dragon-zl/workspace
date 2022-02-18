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

class util_timer;
//定义客户端数据结构体
struct client_data
{
    sockaddr_in address; //保存客户端ip地址
    int sockfd;          //TCP连接套接字
    util_timer *timer;   //定时器指针
};
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire; //设置倒计时时间

    void (*cb_func)(client_data *);
    client_data *user_data; //客户端数据指针
    util_timer *prev;
    util_timer *next;
};
//排序的定时器链表
class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);    //往链表中添加元素
    void adjust_timer(util_timer *timer); //往链表中添加元素，同时保持链表排序不变
    void del_timer(util_timer *timer);    //从链表中，清空一个成员
    void tick();                          //超时处理函数
private:
    void add_timer(util_timer *timer, util_timer *lst_head);
    util_timer *head; //头指针
    util_timer *tail; //尾指针
};
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
};
#endif