#include "threadpool.h"

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