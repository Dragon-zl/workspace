/*
 * @Author       : mark
 * @Date         : 2022-03-01
 */

#include "epoller.h"
/*创建一个 epoll 实例，参数max_size 标识了最大监听数目*/
Epoller::Epoller(int maxEvent):epollFd_(epoll_create(512)), events_(maxEvent){
    assert(epollFd_ >= 0 && events_.size() > 0);
}
/*关闭 epoll 实例*/
Epoller::~Epoller() {
    close(epollFd_);
}
/*向 epollFd_ 注册新的监听的文件描述符和对应的事件*/
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}
/*epollFd_ 修改监听的文件描述符和对应的事件*/
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}
/*epollFd_ 删除监听的文件描述符和对应的事件*/
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

/*epoll 等待就绪的文件描述符*/
int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}
/*获取epoll中就绪文件描述符*/
int Epoller::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

/*获取epoll中就绪事件类型*/
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}