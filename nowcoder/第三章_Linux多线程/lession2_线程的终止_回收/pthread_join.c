/*
    #include <pthread.h>
    int pthread_join(pthread_t thread, void **retval);
        功能： 
            和一个已经终止的线程进行连接
            回收子线程的资源
            特点：
                这个函数时阻塞函数，调用一次只能回收一个子线程
                一般在主线程中使用
        参数：
            thread：线程的ID
            retval：接收子线程退出时的返回值
        返回值：
            成功：0
            失败：返回错误号

*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
int value = 10;
void * callback(void *arg){
    pthread_t tid = pthread_self();
    printf("child thread id : %ld\n" , tid);
    sleep(2);
    //return NULL;
    pthread_exit((void *)&value);
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

    //主线程调用 pthread_join 函数回收子线程资源
    void * retval;
    ret = pthread_join( tid , &retval);
    if( ret != 0 ){
        char * errstr = strerror(ret);
        printf("error:%s\n" , errstr);
    }
    printf("回收子线程资源成功 , retval = %d\n" , *(int *)retval);

    //当主线程调用 pthread_exit(NULL); 退出 ， 主线程退出不会影响 子线的执行
    pthread_exit(NULL);

    return 0;
}