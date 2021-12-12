/*
    #include <pthread.h>

    void pthread_exit(void *retval);
        功能：
            终止一个线程，在哪个线程调用，就终止哪个线程
        参数：
            retval 需要传递一个指针，作为一个返回值，可以在pthread_join()函数中获取
    
    pthread_t pthread_self(void);
        功能：获取当前线程的 tid
    
    int pthread_equal(pthread_t t1, pthread_t t2);
        功能：比较两个 线程 ID 是否相等
        不同的操作系统，pthread_t类型的实现方式不同，这里使用的是 无符号的长整形
        有的使用的是结构体类型实现
*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
void * callback(void *arg){
    pthread_t tid = pthread_self();
    printf("child thread id : %ld\n" , tid);
    return NULL;
}
int main(){
    //1、创建一个线程
    pthread_t tid;
    int ret = pthread_create( &tid , NULL , callback , NULL);
    if( ret != 0 ){
        char * errstr = strerror(ret);
        printf("error:%s\n" , errstr);
    }
    // 打印子线程的 tid 验证与子线程函数的tid是否一致 ， 同时也打印主线程的 id
    printf("child thread tid : %ld , main thread id : %ld\n" , tid , pthread_self());

    //当主线程调用 pthread_exit(NULL); 退出 ， 主线程退出不会影响 子线的执行
    pthread_exit(NULL);

    return 0;
}