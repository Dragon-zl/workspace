#ifndef CLIENT_H
#define CLIENT_H

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>   





class Client{
public:
    Client();
    ~Client();

    void init(int sockFd, const sockaddr_in& addr);

    void Close();
    /*使用 const 限定函数为常成员函数：常成员函数指定函数不能有任何改变其所属对象
        成员变量值的功能*/
    int GetFd() const;
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    int GetPort() const;
    const char* GetIP() const;
    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }
    /*客户端读事件，读取数据后，的逻辑处理函数*/
    bool process();
    
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:

    /*客户端的文件描述符*/
    int fd_;
    /*客户端的IP地址信息*/
    struct  sockaddr_in addr_;
    /*是否关闭客户端标记位*/
    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区
};


#endif