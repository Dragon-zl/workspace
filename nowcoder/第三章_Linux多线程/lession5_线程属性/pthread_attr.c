/*
    pthread_attr_init
        初始化线程属性变量
    pthread_attr_destroy
        释放线程属性变量
    pthread_attr_getdetachstate
        获取线程分离属性
    pthread_attr_setdetachstate
        设置线程分离属性
    pthread_attr_getstacksize
        获取线程栈空间的大小
*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void * callback(void *arg){
    pthread_t tid = pthread_self();
    printf("child thread id : %ld\n" , tid);
    return NULL;
}
int main(){

    //0、设置线程属性
    //创建线程属性变量
    pthread_attr_t  attr;
    //初始化线程属性变量
    pthread_attr_init( &attr );
    //设置 属性
    pthread_attr_setdetachstate( &attr , PTHREAD_CREATE_DETACHED);//设置线程分离属性
    
    //1、创建一个线程
    pthread_t tid;
    int ret = pthread_create( &tid , &attr , callback , NULL);
    if( ret != 0 ){
        char * errstr = strerror(ret);
        printf("error:%s\n" , errstr);
    }

    //获取 线程的栈空间大小
    size_t size;
    pthread_attr_getstacksize( &attr, &size );
    printf("pthread stack size : %ld\n" , size);
    
    // 打印子线程的 tid 验证与子线程函数的tid是否一致 ， 同时也打印主线程的 id
    printf("child thread tid : %ld , main thread id : %ld\n" , tid , pthread_self());

    //设置子线程分离
    //pthread_detach(tid);

    //释放线程属性资源
    pthread_attr_destroy( &attr );
    //当主线程调用 pthread_exit(NULL); 退出 ， 主线程退出不会影响 子线的执行
    pthread_exit(NULL);

    return 0;
}