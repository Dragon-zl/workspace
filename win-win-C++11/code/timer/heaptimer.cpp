/*
 * @Author       : The Lin
 * @Date         : 2021-03-02
 */ 
#include "heaptimer.h"
/*小根堆的上滤*/
void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        /*交换 vector 下标保存的值*/
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}
/*交换堆得两个节点*/
void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
} 
/*小根堆的下滤：将指定节点移动到叶子节点为止*/
bool HeapTimer::siftdown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

/*添加定时器，传入参数 cb 为定时器超时的响应函数*/
void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    /*传入的id为文件描述符*/
    assert(id >= 0);
    size_t i;
    /*通过count查找vector容器中是否有该成员*/
    if(ref_.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i);
    } 
    else {
        /* 已有结点：调整堆 */
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) {
            siftup_(i);
        }
    }
}
/*定时器超时处理函数*/
void HeapTimer::doWork(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}
/*从堆中，删除制定位置的定时器*/
void HeapTimer::del_(size_t index) {
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode_(i, n);
        /*此时，原本队尾的定时器换到了 被删除的定时器位置*/
        /*需要对，从队尾缓过来的节点进行排序调整，先进行上滤，如果上滤失败，进行下滤*/
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    /*从vector末尾 删除元素*/
    heap_.pop_back();
}
/*调整制定定时器时间：重新定时*/
void HeapTimer::adjust(int id, int timeout) {
    /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    /* 重新设置定时器的超时时间 */
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    /* 小根堆的下滤 */
    siftdown_(ref_[id], heap_.size());
}
/*判断是否超时*/
void HeapTimer::tick() {
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        /*函数duration_cast()提供了在不同的时间单位之间进行转换的功能
        判断是否超时
        */
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
    }
}
/*堆首元素移除*/
void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}
/*清空 定时器 vector 和 map heap_*/
void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}
/*超时响应后，判断下一个定时器是否也超时*/
int HeapTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}