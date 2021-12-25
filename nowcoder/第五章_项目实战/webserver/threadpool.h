#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "locker.h"

#include <pthread.h>
#include <list>
#include <exception>
#include <cstdio>
//线程池类，定义成模板类，为了代码的复用 , 模板参数 T 是任务类
template <typename T>
class  threadpool{
    private:
        //线程数量
        int My_thread_number;
        //线程池数组 ， 大小为 My_thread_number
        pthread_t  *  My_threads;
        //请求队列:最多允许等待请求的数量
        int  My_max_requests;
        //请求队列
        std :: list<T*> My_workqueue;
        //互斥锁
        locker My_queuelocker;
        //信号量 ， 用来判断是否有任务要处理
        sem My_queuestat;
        //是否结束线程
        bool My_stop;
    private:
        //线程池工作函数
        static void * worker(void * arg);
        //线程执行函数
        void run();
    public:
        threadpool(int thread_number = 8 , int max_requests = 10000);
        ~threadpool();
        //添加任务
        bool append( T * );
};

#endif