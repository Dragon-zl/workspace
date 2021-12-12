/*
    案例：
        使用多线程实现卖票的过程
        有 3 个窗口，一共是 100 张票
*/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int ticket = 100;//票的数量

void * sellticket(void * arg){
    //卖票
}
int main(){
    //创建 3 个线程
    pthread_t  tid1 , tid2 , tid3;
    pthread_create( &tid1 , NULL , sellticket , NULL);
    pthread_create( &tid2 , NULL , sellticket , NULL);
    pthread_create( &tid3 , NULL , sellticket , NULL);

    //设置线程 分离
    pthread_detach( tid1 );
    pthread_detach( tid2 );
    pthread_detach( tid3 );

    //主线程 退出
    pthread_exit(NULL);
}