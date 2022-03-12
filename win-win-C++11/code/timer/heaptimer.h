/*
 * @Author       : The Lin
 * @Date         : 2021-03-02
 */ 
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    /*计时超时的时间*/
    TimeStamp expires;
    /*cb: 函数指针*/
    TimeoutCallBack cb;
    /*未超时返回真*/
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
class HeapTimer {
public:
                /*  申请内从空间 */
    HeapTimer() { heap_.reserve(64); }
                /*清空 定时器 vector 和 map heap_*/
    ~HeapTimer() { clear(); }
                /*调整制定定时器时间：重新定时*/
    void adjust(int id, int newExpires);
    /*添加定时器，传入参数 cb 为定时器超时的响应函数*/
    void add(int id, int timeOut, const TimeoutCallBack& cb);

    /*定时器超时处理函数*/
    void doWork(int id);

    /*清空 定时器 vector 和 map heap_*/
    void clear();

    /*判断是否超时*/
    void tick();
    /*堆首元素移除*/
    void pop();
    /*超时响应后，判断下一个定时器是否也超时*/
    int GetNextTick();

private:
    /*从堆中，删除制定位置的定时器*/
    void del_(size_t i);

    /*小根堆的上滤*/
    void siftup_(size_t i);

    /*小根堆的下滤：将指定节点移动到叶子节点为止*/
    bool siftdown_(size_t index, size_t n);

    /*交换堆得两个节点*/
    void SwapNode_(size_t i, size_t j);

    /*定时器容器：vector ， 成员类型：TimerNode*/
    std::vector<TimerNode> heap_;

    std::unordered_map<int, size_t> ref_;
};

#endif //HEAP_TIMER_H