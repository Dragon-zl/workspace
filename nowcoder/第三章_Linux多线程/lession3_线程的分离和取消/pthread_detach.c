/*
    #include <pthread.h>
    int pthread_detach(pthread_t thread);  
        功能：分离一个线程
            被分离的线程在终止的时候，会自动释放资源返回给系统
            1、不能多吃分离，会产生不可预料的后果
            2、不能 使用 join去连接已经分离的线程，会报错
        参数：
            thread：线程 ID
        返回值：
            成功：0
            失败：错误号

*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
void * callback(void * arg){
    pthread_t  id = pthread_self();
    printf("child pthread... ID = %ld\n" , id);
    return NULL;
}
int main(){
    pthread_t  tid;
    int ret = pthread_create( &tid , NULL , callback , NULL);
    if( ret != 0){//创建线程失败
        char * errstr = strerror(ret);
        printf("%s\n" , errstr);
    }

    // 打印子线程的 tid 验证与子线程函数的tid是否一致 ， 同时也打印主线程的 id
    printf("child thread tid : %ld , main thread id : %ld\n" , tid , pthread_self());

    pthread_detach(tid);//分离子线程
    pthread_exit(NULL);

    return 0;
}