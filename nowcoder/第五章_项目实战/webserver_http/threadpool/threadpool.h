#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "../lock/locker.h"

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

template <typename T>
threadpool<T> :: threadpool(int thread_number , int max_requests ) : 
    My_thread_number(thread_number),My_max_requests(max_requests),
    My_stop(false) , My_threads(NULL)
{

    if( thread_number <= 0 || max_requests <= 0 ){
        throw std :: exception();
    }
    //动态分配 线程数组
    My_threads = new pthread_t[My_thread_number];
    //判断是否创建成功
    if( !My_threads ){
        throw std :: exception();
    }
    //创建 My_thread_number 个线程 ， 并设置为线程脱离
    for(int i = 0 ; i < My_thread_number ; i++){
        printf("create the %d thread\n" , i);   //打印正在创建第几个 线程
        //由于work函数是静态函数，为了能够让worker函数能够访问 threadpool 类中的成员, 我们参数传递 this 指针
        if( pthread_create( My_threads + i , NULL , worker , this ) != 0 )// worker 函数一定要为静态函数
        {
            delete [] My_threads;
            throw std :: exception();
        }
        //设置线程分离
        if( pthread_detach(My_threads[i]) != 0){
            delete [] My_threads;
            throw std :: exception();
        }   
    }
}
//析构函数
template <typename T>
threadpool<T> :: ~threadpool(){
    delete [] My_threads;
    My_stop = true;
}

//添加任务
template <typename T>
bool threadpool<T> :: append(T * request){
    //上锁
    My_queuelocker.lock();
    //判断请求队列是否已满
    if( My_workqueue.size() >= My_max_requests ){
        //解锁
        My_queuelocker.unlock();
        return false;
    }
    //入队
    My_workqueue.push_back(request);
    //解锁
    My_queuelocker.unlock();
    //信号量 + 1
    My_queuestat.post();
    return true;
}
//worker 函数
template <typename T>
void * threadpool<T> :: worker(void * arg){

    threadpool * pool = (threadpool *)arg;
    pool->run();

    return pool;
}
//run 函数
template <typename T>
void  threadpool<T> :: run(){
    while(!My_stop){
        //信号良 -1
        My_queuestat.wait();
        //上锁
        My_queuelocker.lock();
        //判断请求队列是否为空
        if( My_workqueue.empty()){
            //解锁
            My_queuelocker.unlock();
                continue;
        }
        //从队列头部 获取请求
        T * request = My_workqueue.front();
        //从队列中 头删法删除请求
        My_workqueue.pop_front();
        //解锁
        My_queuelocker.unlock();
        //判断是否获取成功
        if( !request ){
            continue;
        }
        //执行任务函数
        request->process();
    }
}
#endif