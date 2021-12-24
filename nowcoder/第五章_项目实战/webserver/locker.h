#ifndef _LOCKER_H_
#define _LOCKER_H_

#include <pthread.h>
#include <semaphore.h>
#include <exception>

//线程同步机制封装类

//互斥锁类
class  locker{
    public:
        //构造函数
        locker();
        //析构函数
        ~locker();
        //上锁
        bool lock();
        //解锁
        bool unlock();
        //获取锁状态
        pthread_mutex_t * get();
    private:
        pthread_mutex_t  My_mutex;
};
//条件变量类
class  cond{
    public:
        cond();
        ~cond();
        bool wait(pthread_mutex_t * );
        bool time_wait(pthread_mutex_t *  , struct timespec  );
        //唤醒一个条件变量
        bool signal_wait();
        //唤醒所有条件变量
        bool broadcast();
    private:
        pthread_cond_t  My_cond;
};

//信号量类
class  sem{
    public:
        sem();
        sem(int );
        ~sem();
        //等待信号量
        bool  wait();
        //增加信号量
        bool  post();
    private:
        sem_t My_sem;
};



#endif