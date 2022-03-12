/*
 * @Author       : The Lin
 * @Date         : 2022-03-05
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    /*
    explicit: 用于只有一个参数的类构造函数
            作用就是防止类构造函数的隐式自动转换
    */
    /*
                                        make_shared: 比shared_ptr更好，shared_ptr需要维护引用计数的信息
                                                    make_shared可以减少内存分配的次数
                                                    */
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
        /*
            assert：断言函数，判断条件不成立则报错误信息
            */
        assert(threadCount > 0);
        /*
        size_t可能会提高代码的可移植性、有效性或者可读性
        */
        /*
        创建 threadCount 个线程，每个线程为循环判断是否有任务需要处理
       */
        for (size_t i = 0; i < threadCount; i++)
        {
            std::thread([pool = pool_]
            {
                /*加锁*/
                std::unique_lock<std::mutex> locker(pool->mtx);
                while (true)
                {
                    if (!pool->tasks.empty())
                    {
                        /*
                        move()函数：将左值强制转换为右值
                        */
                        auto task = move(pool->tasks.front());
                        pool->tasks.pop();      /*task队列出队*/
                        locker.unlock();
                        task();                 /*执行任务函数*/
                        locker.lock();
                    }
                    else if (pool->isClosed)    /*判断是否关闭所有线程*/
                        break;
                    else
                        pool->cond.wait(locker);/*阻塞 等待条件变量信号*/
                }
            }).detach();/*设置线程分离*/
        }
    }
    /*使用 default ：将函数隐式的声明为内联函数，内联函数减少高频调用时的栈内存*/
    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        /*强制类型转换: static_cast*/
        if(static_cast<bool>(pool_)) {
            {
                /*
                lock_guard类是一个mutex封装者
                lock_guard会在析构函数中释放调锁资源
                */
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }
    /*添加任务到队列中*/
    template<class F>
    void AddTask(F&& task) {
        {
            /*上锁*/
            std::lock_guard<std::mutex> locker(pool_->mtx);
            /*
            emplace比常规的push_back少调用了一次复制构造函数
            高效
            */
            /*
            forward：
           */
            pool_->tasks.emplace(std::forward<F>(task));
        }
        //发出信号
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        /*互斥锁*/
        std::mutex mtx;
        /*条件变量*/
        std::condition_variable cond;
        /*是否关闭线程池标记位*/
        bool isClosed;
        /*任务队列：元素类型为函数指针*/
        std::queue<std::function<void()>> tasks;
    };
    /*智能指针：shared_ptr -- Pool *类型*/
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H