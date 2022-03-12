/*
 * @Author       : The Lin
 * @Date         : 2022-03-10
 */
/*
循环数组实现的阻塞队列，m_back = (m_back + 1)%m_max_size;
线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
*/
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>
template<class T>
class BlockDeque {
public:
    /*explicit 关键字: 只能用于类内部的构造函数声明上
                    : 作用于单个参数的构造函数。
    explicit关键字用来修饰类的构造函数，被修饰的构造函数的类，
    不能发生相应的隐式类型转换
    禁止类对象之间的隐式转换*/

    /*构造函数：初始化阻塞队列的容量*/
    explicit BlockDeque(size_t MaxCapacity = 1000);

    /*清空双端队列：通知生产者、消费之条件变量*/
    ~BlockDeque();

    /*清空双端队列*/
    void clear();

    /*判断双端队列是否为空*/
    bool empty();

    /*判断队列当前元素是否已满*/
    bool full();

    /*清空双端队列：通知生产者、消费之条件变量*/
    void Close();

    /*返回双端队列当前存储的数量*/
    size_t size();

    /*返回双端队列最大容量*/
    size_t capacity();

    /*返回双端队列的头元素*/
    T front();

    /*返回双端队列的尾元素*/
    T back();

    /*从队列尾部插入一元素*/
    void push_back(const T &item);

    /*从队列头部插入一个元素*/
    void push_front(const T &item);

    /*从头部出队一个元素*/
    bool pop(T &item);

    /*从头部出队一个元素，带定时*/
    bool pop(T &item, int timeout);

    /*消费者发送条件变量信号*/
    void flush();

private:
    /*双端队列:用来存储生产者生产的任务*/
    std::deque<T> deq_;

    /*阻塞队列的容量*/
    size_t capacity_;

    /*互斥锁*/
    std::mutex mtx_;

    bool isClose_;

    /*条件变量：
    C++ 11新机制：
        用于线程间同步
        */
    std::condition_variable condConsumer_;

    std::condition_variable condProducer_;
};


template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);
    isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

/*清空双端队列：通知生产者、消费之条件变量*/
template<class T>
void BlockDeque<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
        isClose_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
};

template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();
};

template<class T>
void BlockDeque<T>::clear() {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template<class T>
T BlockDeque<T>::front() {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<class T>
T BlockDeque<T>::back() {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<class T>
size_t BlockDeque<T>::size() {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        condConsumer_.wait(locker);
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) 
                ==std::cv_status::timeout){
            return false;
        }
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

#endif // BLOCKQUEUE_H