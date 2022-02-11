#include "lst_timer.h"
#include "../http/http_conn.h"

sort_timer_lst :: sort_timer_lst(){
    head = NULL;
    tail = NULL;
}
sort_timer_lst :: ~sort_timer_lst(){
    //销毁链表
    util_timer *tmp = head;
    while(tmp){
        head = tmp -> next;
        delete tmp;
        tmp = head;
    }
}
void sort_timer_lst :: add_timer(util_timer *timer){
    //往定时器链表中添加定时器

    if(!timer){
        return;
    }
    if(!head){
        head = tail = timer;
        return;
    }
    //如果刚好比 头节点的超时时间短, 插入头部
    if(timer -> expire < head -> expire){
        timer -> next = head;
        head -> prev = timer;
        head = timer;
        return;
    }
    //否则则调用添加函数
    add_timer(timer , head);
}

void sort_timer_lst :: adjust_timer(util_timer *timer){
    if(!timer){
        return;
    }
    util_timer *tmp = timer -> next;
    if( !tmp || (timer -> expire < tmp -> expire)){
        return;
    }
    if(timer == head){
        head = head -> next;
        head -> prev = NULL;
        timer -> next = NULL;
        add_timer(timer , head);
    }
    else{
        timer -> prev -> next = timer -> next;
        timer -> next -> prev = timer -> prev;
        add_timer(timer , timer -> next);
    }
}
void sort_timer_lst :: del_timer(util_timer * timer){
    if(!timer){
        return; //对传入参数进行错误判断
    }
    //如果 链表就一个节点 , 且要删除这个节点
    if( (timer == head) && (timer == tail)){
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    //删除的是头节点
    if( timer == head){
        head = head -> next;
        head -> prev = NULL;
        delete timer;
        return;
    }
    //删除的是尾节点
    if(timer == tail){
        tail = tail -> prev;
        tail -> next = NULL;
        delete timer;
        return;
    }

    //删除的是中间节点
    timer -> prev -> next = timer -> next;
    timer -> next -> prev = timer -> prev;
    delete timer;
}
void sort_timer_lst :: tick(){
    if(!head){
        return;
    }
    time_t  cur = time(NULL);
    util_timer * tmp = head;
    while(tmp){
        if(cur < tmp -> expire){
            break;
        }
        tmp -> cb_func(tmp -> user_data);
        head = tmp -> next;
        if(head){
            head -> prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}
void sort_timer_lst :: add_timer(util_timer *timer , util_timer * lst_head){
    util_timer * prev = lst_head;
    util_timer * tmp = prev -> next;
    while( tmp ){
        //插入
        if( timer -> expire < tmp -> expire){
            prev -> next = timer;
            timer -> next = tmp;
            tmp -> prev = timer;
            timer -> prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp -> next;
    }
    //超时时间大于等于所有 链表成员,  尾插
    if(!tmp){
        prev -> next = timer;
        timer -> prev = prev;
        timer -> next = NULL;
        tail = timer;
    }
}

void Utils :: init(int timeslot){
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils :: setnonblocking(int fd){
    int old_option = fcntl(fd , F_GETFL);//先获取
    int new_option = old_option | O_NONBLOCK ;//在 或 上非阻塞宏
    fcntl( fd , F_SETFL , new_option);//设置为新的属性
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启 EPOLLONESHOT
void Utils :: addfd(int epollfd , int fd, bool one_shot ,int TRIGMode){
    epoll_event event;
    event.data.fd = fd;

    if(1 == TRIGMode){
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;  //设置为 ET模式
    }
    else{
        event.events = EPOLLIN | EPOLLRDHUP;
    }
    //设置为 ONESHOT 模式
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    //添加事件属性
    epoll_ctl(epollfd , EPOLL_CTL_ADD , fd , &event);
    //设置为非阻塞
    setnonblocking(fd);
}

//设置信号响应的 处理函数
void Utils :: addsig(int sig , void(handler)(int), bool restart){
    struct sigaction sa;
    memset(&sa , '\0' , sizeof(sa));//清空(置零)
    sa.sa_handler = handler;
    if(restart){
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig , &sa , NULL) != -1);
}

//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}
//定时器处理任务 : 重新定时以不断触发 SIGALRM 信号
void Utils :: timer_handler(){
    m_timer_lst.tick(); //定时
    alarm(m_TIMESLOT);
}

void Utils :: show_error(int connfd , const char * info){
    send(connfd , info , strlen(info) , 0);
    close(connfd);
}

int * Utils :: u_pipefd = 0;
int Utils :: u_epollfd = 0;

void cb_func(client_data * user_data){
    epoll_ctl(Utils :: u_epollfd ,EPOLL_CTL_DEL , user_data -> sockfd , 0);
    assert(user_data);
    close(user_data -> sockfd);
    http_conn :: My_users_count --;
}