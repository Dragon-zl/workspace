/*
    #include <pthread.h>

    int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                        void *(*start_routine) (void *), void *arg);
        功能：
            创建一个子线程
        参数：
            thread ：(传出参数， 线程创建成功后，子线程的线程ID保存在此参数中)
            attr：设置线程的属性，一般使用默认值，传递 NULL
            start_routine：函数指针
                这个函数是子线程需要处理的逻辑代码
            arg：给第三个参数传参
        返回值：
            成功：0
            失败：返回错误号，这个错误号和之前的errno不一样
                获取错误号信息：
                    char * strerror(int errnum);

*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
void * callback(void * arg){
    printf("child  pthread... arg = %d\n" , *(int *)arg);
    return NULL;
}
int main(){
    pthread_t  tid;
    int arg = 10;
    int ret = pthread_create( &tid, NULL, callback, (void *)&arg);
    if( ret != 0 ){
        //创建失败
        char * errstr = strerror(ret);
        printf("%s\n" , errstr);
    }
    sleep(1);
    return 0;
}