/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

/*写索引 - 读索引 = 读到的数据量*/
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
/*容器总的数据量 - 写索引 = 可用于读或写的数据量空间*/
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}
/*返回读索引*/
size_t Buffer::PrependableBytes() const {
    return readPos_;
}
/*读索引 + 读写容器的首地址 = 读索引的相对位置*/
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}
/*读完数据后 读索引 右移*/
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}
/*将读索引 右移*/
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}
/*清空读写容器内的所有数据，读写索引都归零*/
void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}
/*将读到的数据char 类型转化为 string类型*/
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}
/*返回写索引相对容器首地址的位置*/
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}
/*返回写索引相对容器首地址的位置*/
char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}
/*写索引 右移*/
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

/*将string字符串数组 复制到写缓冲容器*/
void Buffer::Append(const std::string& str) {
    /*data函数：返回sting字符串数组的首地址*/
    Append(str.data(), str.length());
}
/*将string字符串数组 复制到写缓冲容器*/
void Buffer::Append(const void* data, size_t len) {
    assert(data);
    /*强制类型转换*/
    Append(static_cast<const char*>(data), len);
}
/*将string字符串数组 复制到写缓冲容器*/
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    /*判断是否有足够的写缓冲容器：没有则扩容*/
    EnsureWriteable(len);
    /*将string 字符串，复制到读写容器上*/
    std::copy(str, str + len, BeginWrite());
    /*将写索引 右移*/
    HasWritten(len);
}
/*将string字符串数组 复制到写缓冲容器*/
void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

/*判断是否有足够的存储发送数据量的容器空间，没有则扩容*/
void Buffer::EnsureWriteable(size_t len) {
    /*比较可以发送的数据空间 和 len的大小*/
    if(WritableBytes() < len) {
        /*不够：则进行扩容*/
        MakeSpace_(len);
    }
    /*如果有足够的发送区空间*/
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
/*数据发送*/
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    /*返回可以发送的数据量*/
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}
/*返回buffer容器的首地址*/
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}
/*返回读写存储容器的首地址*/
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