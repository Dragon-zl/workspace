/*
    #include <pthread.h>
    int pthread_cancel(pthread_t thread);
        功能：取消(终止)一个线程
                这个终止过程并不是立马终止
                而是当子线程执行到一个取消点时，线程才会取消
                取消点：
                    系统规定的一些系统调用，我们可以粗略的理解为从用户态到系统态的切换
*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
void * callback(void *arg){
    pthread_t tid = pthread_self();
    printf("child thread id : %ld\n" , tid);
    
    for( int i = 0 ; i < 10 ; i++){
        printf("child : %d\n" , i);
    }
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

    //取消线程
    pthread_cancel(tid);

    // 打印子线程的 tid 验证与子线程函数的tid是否一致 ， 同时也打印主线程的 id
    printf("child thread tid : %ld , main thread id : %ld\n" , tid , pthread_self());

    


    //当主线程调用 pthread_exit(NULL); 退出 ， 主线程退出不会影响 子线的执行
    pthread_exit(NULL);

    return 0;
}