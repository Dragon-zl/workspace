#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

/*
循环数组实现的阻塞队列，m_back = (m_back + 1)%m_max_size;
线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
*/
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"
using namespace std;

template <class T>
class block_queue
{
public:
    //构造函数：默认队列最大容量为 1000
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)//对传入参数进行错误判断
        {
            exit(-1);
        }
        m_max_size = max_size;//最大容量
        m_array = new T[max_size];//创建模板数组队列
        m_size = 0;//当前队列元素个数
        m_front = -1;//队首元素
        m_back = -1;//队尾元素
    }
    //析构函数
    ~block_queue()
    {
        m_mutex.lock();
        if (m_array != NULL) //将模板数组队列释放掉
            delete[] m_array;
        m_mutex.unlock();
    }
    //清空
    void clear(){
        m_mutex.lock();
        m_size = 0;//将当前元素个数、队首元素下标、队尾元素下标 复位
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }
    //判断队列是否满了
    //满了: true  没满：false
    bool full(){
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    //判断阻塞队列是否为空
    //空 ：返回 true 不为空：false
    bool empty(){
        m_mutex.lock();
        
        if (0 == m_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    //返回队首元素
    //成功 ：true  失败: false
    bool front(T &value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }
    //返回队尾元素
    bool back(T &value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }
    //返回队列当前元素个数
    int size()
    {
        int tmp = 0;
        m_mutex.lock();
        tmp = m_size;
        m_mutex.unlock();
        return tmp;
    }
    //返回队列最大容量
    int max_size()
    {
        int tmp = 0;
        m_mutex.lock();
        tmp = m_max_size;
        m_mutex.unlock();
        return tmp;
    }
    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列，相当于生产者生产了一个元素
    //若当前没有线程等待条件变量，则唤醒无意义
    bool push(const T &item){
        m_mutex.lock();
        //判断队列当前元素个数，是否 >= 最大容量
        if (m_size >= m_max_size)
        {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;
        m_cond.broadcast(); //广播给所有条件变量发信号
        m_mutex.unlock();
    }
    //出队(无超时处理)
    bool pop(T &item){
        m_mutex.lock();
        //如果当前队列没有元素
        while (m_size <= 0)
        {   //条件变量等待信号
            if (!m_cond.wait(m_mutex.get()))
            {
                m_mutex.unlock();
                return false;
            }
        }
        m_front = (m_front + 1) % m_max_size;//队首元素下标 后移
        item = m_array[m_front];
        m_size--;               //队列当前元素个数 --
        m_mutex.unlock();
        return true;
    }
    //出队(增加超时处理)
    bool pop(T &item, int ms_timeout){
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);//获取当前时间
        m_mutex.lock();
        //如果当前队列没有元素
        if (m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t))
            {   //将传入的定时时间，转为结构体类型，条件变量(定时等待)
                m_mutex.unlock();
                return false;
            }
        }
        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }
        //出队
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

private:
    locker m_mutex;
    cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif