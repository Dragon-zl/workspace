/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
/*返回可以写的数据*/
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::PrependableBytes() const {
    return readPos_;
}
/*返回为发送的数据的首地址*/
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}
/*buff扩容函数
参数：
    str：buff的首地址
    len：需要扩容的字节数
*/
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}
/*读事件：读取数据函数
    参数：saveErrno 传出参数，保存读取异常的信息
*/
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];
    /*定义 struct iovec 结构体数组，用于 readv 读取数据*/
    struct iovec iov[2];
    /*writable： 表示 buff容器剩余的可以读的空间*/
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    /*保存当前，读取数据容器的起始位置*/
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    /*第二个成员：用于灵活扩容buff*/
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    /*调用 readv 函数进行读取*/
    const ssize_t len = readv(fd, iov, 2);
    /*如果调用失败*/
    if(len < 0) {
        *saveErrno = errno;
    }
    /*如果读取到的字节数 <= writable： 不需要扩容*/
    else if(static_cast<size_t>(len) <= writable) {
        /*
        读到的字节数递增
        */
        writePos_ += len;
    }
    else {
        /*
        意味着需要读容器需要扩容
        */
        writePos_ = buffer_.size();
        /*扩容*/
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    /*保存为发送出去的数据*/
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}
/*返回buffer容器当前的下标*/
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}
/*返回buffer的首地址*/
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        /*调用 resize 函数 队buffer进行扩容*/
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}