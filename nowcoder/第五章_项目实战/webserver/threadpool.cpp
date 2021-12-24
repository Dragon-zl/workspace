#include "threadpool.h"
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
    public:
        threadpool(int thread_number = 8 , int max_requests = 10000);
        ~threadpool();

        //添加任务
        bool append( T * request);
};
template <typename T>
threadpool<T> :: threadpool(int thread_number , int max_requests ) : 
    My_thread_number(thread_number),My_max_requests(max_requests),
    My_stop(stop) , My_threads(NULL){

        if(thread_number <= 0 || max_requests <= 0 ){
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
            
            if( pthread_create( My_threads + i , NULL , worker , NULL) != 0 )// worker 函数一定要为静态函数
            {
                delete [] My_threads;
                throw std :: exception();
            }
            //设置线程分离
            if( pthread_detach(My_threads[i] != 0){
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
    
}