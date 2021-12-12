/*
    互斥量的类型：pthread_mutex_t
    #include <pthread.h>

    int pthread_mutex_init(pthread_mutex_t *restrict mutex,
        const pthread_mutexattr_t *restrict attr);  
        功能：初始化互斥量
        参数：
            mutex ：需要初始化的互斥量变量
            attr：互斥量的属性，NULL
        restrict：
            C语言的修饰符，被他修饰的指针，不能由另外的一个指针进行操作
    
    int pthread_mutex_destroy(pthread_mutex_t *mutex);
        释放互斥量的资源
    
    int pthread_mutex_lock(pthread_mutex_t *mutex);
        功能：
            加锁，阻塞的，如果有一个线程加锁了，那么其他的线程只能阻塞等待
    int pthread_mutex_trylock(pthread_mutex_t *mutex);
        功能：
            尝试加锁，如果加锁失败，不会阻塞，直接返回
    int pthread_mutex_unlock(pthread_mutex_t *mutex);
        解锁
*/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int ticket = 100;//票的数量

//创建一个互斥量
pthread_mutex_t  mutex;

void * sellticket(void * arg){
    //卖票
    while(1){
        usleep(5000);
        //上锁
        pthread_mutex_lock(&mutex);
        if(ticket > 0){
            printf("%ld 正在卖票 第 %d 张门票\n" , pthread_self()  , ticket); 
            ticket--;
        }
        else{
            //解锁
            pthread_mutex_unlock(&mutex);
            break;
        }
        //解锁
        pthread_mutex_unlock(&mutex);
    }
}
int main(){

    //初始化互斥量
    pthread_mutex_init(&mutex , NULL);
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

    //释放互斥量
    pthread_mutex_destroy(&mutex);
}