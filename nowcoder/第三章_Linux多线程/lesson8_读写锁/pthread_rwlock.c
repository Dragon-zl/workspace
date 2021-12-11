/*
    读写锁的类型：
        pthread_rwlock_t
    API函数：
        pthread_rwlockattr_init
        pthread_rwlock_destroy
        pthread_rwlock_rdlock
        pthread_rwlock_tryrdlock
        pthread_rwlock_wrlock
        pthread_rwlock_trywrlock
        pthread_rwlock_unlock  //解锁函数只有这一个
*/
/*
    案例：
        创建 8 个线程
        2 个线程 对全局变量进行写操作， 6 个线程进行 读操作
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
int num = 1;
//创建读写索 变量
pthread_rwlock_t  rwlock;
//写操作函数
void * writeNum(void * arg){
    while(1){
        //加 写锁
        pthread_rwlock_wrlock( &rwlock );
        if( num < 1000){
            printf("write pthread tid: %ld , ++num : %d\n" , pthread_self() , ++num);
        }
        else{
            //解锁
            pthread_rwlock_unlock( &rwlock );
            break;
        }
        //解锁
        pthread_rwlock_unlock( &rwlock );
        usleep(2000);
    }
    return NULL;
}
//读操作函数
void * readNum(void * arg){
    while(1){
        //加读锁
        pthread_rwlock_rdlock( &rwlock );
        if( num < 1000){
            printf("read  pthread tid: %ld ,   num : %d\n" , pthread_self() , num);
        }
        else{
            //解锁
            pthread_rwlock_unlock( &rwlock );
            break;
        }
        //解锁
        pthread_rwlock_unlock( &rwlock );
        usleep(2000);
    }
    return NULL;
}
int main(){
    //初始化读写锁
    pthread_rwlock_init( &rwlock , NULL);

    pthread_t  wtids[2] , rtids[6];
    for( int i = 0 ; i < 2 ; i++){
        //创建线程
        pthread_create( &wtids[i] , NULL , writeNum , NULL);
        //设置线程分离
        pthread_detach( wtids[i] );
    }
    for( int i = 0 ; i < 6 ; i++){
        pthread_create( &rtids[i] , NULL , readNum , NULL);
        pthread_detach( rtids[i] );
    }
    //主线程正常退出
    pthread_exit(NULL);

    //释放读写锁
    pthread_rwlock_unlock( &rwlock );
}



