#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool{
public:
    /*
    thread_number是线程池中线程的数量，max_requests是请求队列中
    最多允许的、等待处理的请求的数量
    */
    threadpool(int actor_model, connection_pool *connPool,
                int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T *request, int state);
    bool append_p(T *request);
private:
    /*
    工作线程运行的函数，它不断从工作队列中取出任务并执行之
    */
   static void * worker(void *arg);
   void run();
private:
    int m_thread_number;            //线程池中的线程数
    int m_max_requests;             //请求队列中允许的最大请求数
    pthread_t *m_threads;           //描述线程池的数组，其长度(容量)为m_thread_number
    std::list<T*> m_workqueue;      //请求队列
    locker m_queuelocker;           //保护请求队列的互斥锁
    sem m_queuestat;                //是否有任务需要处理
    connection_pool *m_connPool;    //数据库
    int m_actor_model;              //模型切换
};
template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connPool,
                int thread_number , int max_requests):m_actor_model(actor_model),
                m_thread_number(thread_number), m_max_requests(max_requests),
                m_threads(NULL),m_connPool(connPool)
{
    //错误传入参数判断
    if(thread_number <= 0 || max_requests <= 0){
        throw std::exception();
    }
    //创建线程池数组
    m_threads = new pthread_t[m_thread_number];
    //如果内存不足，可能会创建失败
    if(!m_threads){
        throw std::exception();
    }
    //遍历线程池数组，对每个成员进行 创建线程实例
    for(int i = 0; i < thread_number; ++i){
        
        if(pthread_create(m_threads + i, NULL, worker, this) != 0){
            //如果创建失败
            delete[] m_threads; //释放掉线程池数组
            throw std::exception();
        }
        //设置线程分离，即线程结束后，自动回收
        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template <typename T>
threadpool<T>::~threadpool(){//析构函数，清空线程数组
    delete[] m_threads;
} 
template <typename T>   //往工作队列中添加任务
bool threadpool<T>::append(T *request, int state){
    //加锁
    m_queuelocker.lock();
    //判断请求队列是否已满
    if(m_workqueue.size() >= m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    request -> m_state = state;
    m_workqueue.push_back(request);//入队
    m_queuelocker.unlock();//解锁
    m_queuestat.post(); //信号量发送信号给工作线程
    return true;
}
template <typename T>
bool threadpool<T>::append_p(T *request){
    //加锁
    m_queuelocker.lock();
    if(m_workqueue.size() >= m_max_requests){
        //判断请求队列是否已满
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);//入队
    m_queuelocker.unlock();//解锁
    m_queuestat.post();//信号量发送信号给工作线程
    return true;
}
template <typename T>   //线程池工作线程
void * threadpool<T>::worker(void *arg){
    threadpool *pool = (threadpool *)arg;
    pool -> run();
    return pool;
}
template <typename T>
void threadpool<T>::run(){
    while(true){
        //信号量，等待任务入队
        m_queuestat.wait();
        //上锁
        m_queuelocker.lock();
        //判断工作队列是否为空
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        //任务出队
        T * request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();//解锁
        if(!request){
            continue;
        }
        //线程池工作模式：reactor 模式 (即线程自己去处理读、写事件的读、写数据)
        if(1 == m_actor_model){
            if(0 == request -> m_state){
                if(request -> read_once()){
                    request -> improv = 1;
                    connectionRAII mysqlcon(&request -> mysql,
                                            m_connPool);
                    request -> process();
                }
                else{
                    
                    request -> improv = 1;
                    request -> timer_flag = 1;
                }
            }
            else{
                if(request -> m_write()){
                    request -> improv = 1;
                }
                else{
                    request -> improv = 1;
                    request -> timer_flag = 1;
                }
            }
        }
        //proactor 模式：主函数处理了读写事件的读数据、写数据，线程只需要处理数据
        else{
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            request -> process();
        }
    }
}
#endif