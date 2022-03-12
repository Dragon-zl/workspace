#include "client.h"

std::atomic<int> Client::userCount;
bool Client::isET;
const char* Client::srcDir;

void Client::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

int Client::GetFd() const {
    return fd_;
};
int Client::GetPort() const {
    return addr_.sin_port;
}
const char* Client::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}
/*客户端读事件，读取数据后，的逻辑处理函数*/
bool Client::process() {
    std::string clientdate = readBuff_.RetrieveAllToStr();

    std::cout << clientdate << std::endl;

    LOG_DEBUG("filesize:%d, %d  to %d" , iovCnt_, ToWriteBytes());
    return false;//返回 FALSE 重新注册读事件
}
void Client::Close() {
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

ssize_t Client::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}
ssize_t Client::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}
Client::Client() { 
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};
Client::~Client() { 
    Close(); 
};
