# 线程池

## 说明

```c++
1、本线程池，采用一个模板链表，作为任务请求队列
2、使用数组的形式，存储多个工作线程
```

## 工作模式

```c++
1、有任务时
    将任务添加到请求队列，通过信号量，给空闲的线程发信号
2、空闲的线程收到信号
    空闲的线程收到信号，会从队列中取出一个任务，(采用互斥锁保护队列)，按照 m_actor_model 对应的工作模式，进行相应的函数调度工作。
    
```

