#include "lst_timer.h"

//构造函数，头尾指针都为NULL
sort_timer_lst :: sort_timer_lst(){
    head = NULL;
    tail = NULL;
}

sort_timer_lst :: ~sort_timer_lst(){
    util_timer * tmp = head;
    //清空定时器链表
    while(tmp){
        head = tmp -> next;
        delete tmp;
        tmp = head;
    }
}
//往链表中添加元素
void sort_timer_lst :: add_timer(util_timer *timer){
    if(!timer){
        return;
    }
    if(!head){
        head = tail = NULL;
        return;
    }
    //如果新的定时器超时时间小于当前头部结点
    //直接将当前定时器结点作为头部结点
    if(timer -> expire < head -> expire){
        timer -> next = head;
        head -> prev = timer;
        head = timer;
        return;
    }
    //否则调用私有成员，调整内部结点
    add_timer(timer , head);
}
//调整定时器，任务发生变化时，调整定时器在链表中的位置
void sort_timer_lst :: adjust_timer(util_timer *timer){
    if(!timer){
        return;
    }
    util_timer *tmp = timer -> next;
    //被调整的定时器在链表尾部
    //定时器超时值仍然小于下一个定时器超时值，不调整
    if(!tmp || (timer -> expire < tmp -> expire)){
        return;
    }
    //被调整定时器是链表头结点，将定时器取出，重新插入
    if(timer == head){
        head = head -> next;
        head -> prev = NULL;
        timer -> next = NULL;
        add_timer(timer, head);
    }
    //被调整定时器在内部，将定时器取出，重新插入
    else{
        timer -> prev -> next = timer -> next;
        timer -> next -> prev = timer -> prev;
        add_timer(timer , timer -> next);
    }
}
//删除定时器
void sort_timer_lst :: del_timer(util_timer *timer){
    if (!timer)
    {
        return;
    }
    //链表中只有一个定时器，需要删除该定时器
    if ((timer == head) && (timer == tail))
    {
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    //被删除的定时器为头结点
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    //被删除的定时器为尾结点
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    //被删除的定时器在链表内部，常规链表结点删除
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}
//定时任务处理函数
void sort_timer_lst::tick(){
    //遍历定时器升序链表容器，从头结点开始依次处理每个定时器，直到遇到尚未到期的定时器
    if(!head){
        return;
    }
    time_t cur = time(NULL);//获取当前时间
    util_timer *tmp = head;
    //遍历定时器链表
    while (tmp)
    {
        //链表容器为升序排列
        //当前时间小于定时器的超时时间，后面的定时器也没有到期
        if (cur < tmp->expire)
        {
            break;
        }
        //当前定时器到期，则调用回调函数，执行定时事件
        tmp->cb_func(tmp->user_data);
        //将处理后的定时器从链表容器中删除，并重置头结点
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}
//将节点插入排序好的链表，并保持链表有序
void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head){
    util_timer *prev = lst_head;//遍历时，保存上一个节点
    util_timer *tmp = prev -> next;//临时节点，用来遍历链表
    //遍历当前结点之后的链表，按照超时时间找到目标定时器对应的位置，常规双向链表插入操作
    while(tmp){
        if(timer -> expire < tmp -> expire){
            prev -> next = timer;
            timer -> next = tmp;
            tmp -> prev = timer;
            timer -> prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp -> next;
    }
    //遍历完发现，目标定时器需要放到尾结点处
    if(!tmp){
        prev -> next = timer;
        timer -> prev = prev;
        timer -> next = NULL;
        tail = timer;
    }
}
//初始化
void Utils::init(int timeslot){
    m_TIMESLOT = timeslot;
}
//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);//获取
    int new_option = old_option | O_NONBLOCK;// | 上非阻塞宏
    fcntl(fd, F_SETFL, new_option);//设置
    return old_option;
}
//间隔内核事件表注册 读事件、ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;
    // == 1设置为 ET模式
    if(1 == TRIGMode){
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    // 默认LT模式
    else{
        event.events = EPOLLIN | EPOLLRDHUP;
    }
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
//信号处理函数
//信号处理函数中仅仅通过管道发送信号值，
//不处理信号对应的逻辑，缩短异步执行时间，减少对主程序的影响
void Utils::sig_handler(int sig){
    //为保证函数的可重入性，保留原来的errno
    //可重入性表示中断后再次进入该函数，环境变量与之前相同，不会丢失数据
    int save_errno = errno;
    int msg = sig;
    //将信号值从管道写端写入，传输字符类型，而非整型
    send(u_pipefd[1], (char *)&msg, 1, 0);
    //将原来的errno赋值为当前的errno
    errno = save_errno;
}
//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;//设置信号处理函数
    if(restart){
        //指定信号处理的行为
        sa.sa_flags |= SA_RESTART;//被信号打断的系统调用自动重新发起
    }
    sigfillset(&sa.sa_mask);//用来将参数set信号集初始化，然后把所有的信号加入到此信号集里。
    assert(sigaction(sig, &sa, NULL) != -1);
}
//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler(){
    m_timer_lst.tick();
    //设置信号SIGALRM在经过m_TIMESLOT发送给当前进程
    alarm(m_TIMESLOT);
}
    
void Utils::show_error(int connfd, const char *info){
    send(connfd, info, strlen(info), 0);
    close(connfd);
}


int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;
class Utils;
//定时器回调函数
void cb_func(client_data *user_data){
    //删除非活动连接在socket上的注册事件
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    //关闭文件描述符
    close(user_data->sockfd);
    
    //减少连接数
    TcpClient::m_user_count--;
}