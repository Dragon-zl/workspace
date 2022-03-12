/*
 * @Author       : The Lin
 * @Date         : 2022-03-02
 */ 
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
public:
    /*创建一个 epoll 实例，参数 maxEvent 标识了最大监听数目*/
    explicit Epoller(int maxEvent = 1024);

    /*关闭 epoll 实例*/
    ~Epoller();

    /*向 epollFd_ 添加监听的文件描述符和对应的事件*/
    bool AddFd(int fd, uint32_t events);

    /*epollFd_ 修改监听的文件描述符和对应的事件*/
    bool ModFd(int fd, uint32_t events);

    /*epollFd_ 删除监听的文件描述符和对应的事件*/
    bool DelFd(int fd);

    /*epoll 等待就绪的文件描述符，参数timeoutMs = -1 ， 默认为阻塞等待*/
    int Wait(int timeoutMs = -1);

    /*获取epoll中就绪文件描述符*/
    int GetEventFd(size_t i) const;

    /*获取epoll中就绪事件类型*/
    uint32_t GetEvents(size_t i) const;
        
private:
    int epollFd_;

    std::vector<struct epoll_event> events_;    
};

#endif //EPOLLER_H