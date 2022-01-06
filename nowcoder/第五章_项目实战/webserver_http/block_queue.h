#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <sys/time.h>
#include "locker.h"
using namespace std;


template <class T>
class  block_queue
{
    public:
        block_queue(int max_size = 10000){
            if(max_size <= 0){
                exit(-1);
            }
            //设置请求队列个数
            My_max_size = max_size;
            //创建队列数组
            m_array = new T[max_size];
            My_size = 0;
            My_front = -1;
            My_back = -1;
            
        }
        ~block_queue(){
            My_mutex.lock();
            //清空用户数组
            if(m_array != NULL){
                deleet [] m_array;
            }
            My_mutex.unlock();
        }
        //清空
        void  clear(){
            //上锁
            My_mutex.lock();
            My_size = 0;
            My_front = -1;
            My_back = -1;
            //解锁
            My_mutex.unlock();
        }
        //判断队列是否满了
        bool  full(){
            My_mutex.lock();
            //判断请求个数是否超值
            if( My_size >= My_max_size){
                My_mutex.unlock();
                return true;
            }
            My_mutex.unlock();
            return false;
        }
        //判断队列是否为空
        bool  empty(){
            My_mutex.lock();
            if( 0 == My_size){
                My_mutex.unlock();
                return true;
            }
            My_mutex.unlock();
            return false;
        }
        //返回队首元素
        bool  front(T  &value){
            //传入引用
            My_mutex.lock();
            //判断如果为空队列
            if(0 == My_size){
                My_mutex.unlock();
                return false;
            }
            value = m_array[My_front];
            My_mutex.unlock();
            return true;
        }
        //返回队尾元素
        bool back(T &value){
            My_mutex.lock();
            if( 0 == My_size){
                My_mutex.unlock();
                return false;
            }
            value = m_array[back];
            My_mutex.unlock();
            return true;
        }
        //返回队列个数
        int size(){
            int tmp = 0;
            My_mutex.lock();
            tmp = My_size;
            My_mutex.unlock();
            return tmp;
        }
        //返回最大待处理队列个数
        int  max_size(){
            int tmp;
            My_mutex.lock();
            tmp = My_max_size;
            My_mutex.unlock();
            return tmp;
        }
        //往队列添加元素，需要将所有使用队列的线程先唤醒
        //当有元素 push 进队列，相当于生产者生产了一个元素
        //若当前没有线程等待条件变量，则唤醒无意义
        bool push(const T &item){
            //上锁
            My_mutex.lock();
            //如果入队前判断队列元素已满 , 入队失败
            if(My_size >= My_max_size){
                //唤醒所有线程
                My_cond.broadcast();
                My_mutex.unlock();
                return false;
            }
            //计算队尾元素的数组下标
            My_back = (My_back + 1) % My_max_size;
            //尾插法
            m_array[My_back] = item;
            My_size++;
            //唤醒所有线程
            My_cond.broadcast();
            //解锁
            My_mutex.unlock();
            return true;
        }
        //出队:如果当前队列没有元素，将会等待条件变量
        bool pop( T &item){
            My_mutex.lock();
            while( My_size <= 0){
                if(!My_cond.wait(My_mutex.get())){
                    My_mutex.unlock();
                    return false;
                }
            }
            //计算出队元素的下标
            My_front = (My_front + 1) % My_max_size;
            //头删法
            item = m_array[My_front];
            My_size--;
            My_mutex.unlock();
            return true;
        }
        //增加了超时处理的出队函数
        bool pop(T &item , int ms_timeout){
            struct timespec t = {0 , 0};
            struct timeval now = {0 , 0};
            //获取当前时间
            gettimeofday( &now , NULL);
            //上锁
            My_mutex.lock();
            if(My_size <= 0){
                t.tv_sec = now.tv_sec + ms_timeout / 1000;
                t.tv_nsec = (ms_timeout % 1000) * 1000;
                if(!My_cond.time_wait(My_mutex.get() , t)){
                    My_mutex.unlock();
                    return false;
                }
            }
            if(My_size <= 0){
                My_mutex.unlock();
                return false;
            }
            //计算出队元素的下标
            My_front = (My_front + 1) % My_max_size;
            //头删法
            item = m_array[My_front];
            My_size--;
            My_mutex.unlock();
            return true;
        }
    private:
        //最大未处理请求队列个数
        int My_max_size;
        //请求队列个数
        int My_size;
        int My_front;
        int My_back;
        T * m_array;
        //-----------
        //互斥锁
        locker My_mutex;
        //信号量
        cond  My_cond;

};

#endif