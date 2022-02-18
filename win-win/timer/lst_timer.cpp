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
    //如果插入的定时器的时间比头节点还小
    if(timer -> expire < head -> expire){
        timer -> next = head;
        head -> prev = timer;
        head = timer;
        return;
    }
    add_timer(timer , head);
}

void sort_timer_lst :: adjust_timer(util_timer *timer){
    if(!timer){
        return;
    }
    util_timer *tmp = timer -> next;
    //如果传入的链表只有一个节点，或者以及是从小到大的顺序排列
    if(!tmp || (timer -> expire < tmp -> expire)){
        return;
    }
    if(timer == head){
        head = head -> next;
        head -> prev = NULL;
        timer -> next = NULL;
        add_timer(timer, head);
    }
    else{
        //将当前节点，从链表中取出
        timer -> prev -> next = timer -> next;
        timer -> next -> prev = timer -> prev;
        add_timer(timer , timer -> next);
    }
}
//删除一个节点
void sort_timer_lst :: del_timer(util_timer *timer){
    if (!timer)
    {
        return;
    }
    //如果链表只有一个节点
    if ((timer == head) && (timer == tail))
    {
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    //如果要删除的节点是，头节点
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    //如果要删除的节点是，尾节点
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}
void sort_timer_lst::tick(){
    if(!head){
        return;
    }
    time_t cur = time(NULL);//获取当前时间
    util_timer *tmp = head;
    while (tmp)
    {
        
        if (cur < tmp->expire)
        {
            break;
        }
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;
class Utils;
void cb_func(client_data *user_data){
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
}