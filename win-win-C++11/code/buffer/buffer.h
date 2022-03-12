/*
 * @Author       : The Lin
 * @Date         : 2022-03-11
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    /*TCP/ip 通信默认读写容器大小 ： 1024*/
    Buffer(int initBuffSize = 1024);
    /*使用 default ：将函数隐式的声明为内联函数，内联函数减少高频调用时的栈内存*/
    ~Buffer() = default;

    /*使用 const 限定函数为常成员函数：常成员函数指定函数不能有任何改变其所属对象
        成员变量值的功能*/
    /*返回可用于写的数据量空间*/
    size_t WritableBytes() const;       
    /*返回读到的数据量*/
    size_t ReadableBytes() const ;
    /*返回读索引*/
    size_t PrependableBytes() const;

    /*使用 const修饰返回值：保护指针指向的内容不会被修改*/

    /*返回读索引的相对读写容器首地址的位置*/
    const char* Peek() const;
    /*使用size_t：方便不同系统之间的移植*/

    /*判断是否有足够的存储发送数据量的容器空间，没有则扩容*/
    void EnsureWriteable(size_t len);

    /*发送完数据后写索引 右移*/
    void HasWritten(size_t len);

    /*读完数据后 读索引 右移*/
    void Retrieve(size_t len);

    /*将读索引 右移*/
    void RetrieveUntil(const char* end);

    /*清空读写容器内的所有数据，读写索引都归零*/
    void RetrieveAll() ;

    /*将读到的数据转化为 string 返回，并清空容器，读写索引归零*/
    std::string RetrieveAllToStr();

    /*返回写索引相对容器首地址的位置*/
    const char* BeginWriteConst() const;

    /*返回写索引相对容器首地址的位置*/
    char* BeginWrite();

    /*将string字符串数组 复制到写缓冲容器*/
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    /*套接字数据读取*/
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    /*返回buffer容器的首地址*/
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    /*
    读写：用同一个容器
    */
    std::vector<char> buffer_;
    /*
    atomic: 原子数据类型：原子数据类型不会发生数据竞争，
            能直接用在多线程中而不必我们用户对其进行添加互斥资源锁的类型。
    */
    std::atomic<std::size_t> readPos_;/*读容器的索引*/
    std::atomic<std::size_t> writePos_;/*写容器的索引*/
    /*不管是读取到的数据，还是需要发送的数据，数据的左边界为 readPos_ 右边界为 writePos_*/
};

#endif //BUFFER_H