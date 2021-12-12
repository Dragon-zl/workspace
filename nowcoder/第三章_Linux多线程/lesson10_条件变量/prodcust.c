/*
    案例：
        生产者、消费者模型(粗略的版本)
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
//互斥量
pthread_mutex_t  mutex;
struct  Node{
    int num;
    struct  Node * next;
};
//头节点
struct Node *  head = NULL;

//生产者函数
void * producer(void * arg){
    
    //头插法：往连表内插入节点
    while(1){
        //加锁
        pthread_mutex_lock( &mutex );
        struct  Node *  node = (struct  Node * )malloc(sizeof(struct  Node));
        node->num = rand() % 1000;  //取随机值
        node -> next = head;
        head = node;    //头插
        printf("add node, num: %d, tid: %ld\n" , node->num , pthread_self());
        //解锁
        pthread_mutex_unlock( &mutex );
        usleep(100);
    }
    return NULL;
}
//消费者函数
void * customer( void * arg){
    
    //头删法
    while(1){
        //加锁
        pthread_mutex_lock( &mutex );
        if( head != NULL){
            //先保存头节点
            struct Node * node = head;
            head = head -> next;
            printf("del node, num: %d, tid: %ld\n" , node->num , pthread_self());
            free(node);
            //解锁
            pthread_mutex_unlock( &mutex );
        
        }else{
            //解锁
            pthread_mutex_unlock( &mutex );
        }
        usleep(100);
    }
    return NULL;
}
int main(){
    //初始化互斥量
    pthread_mutex_init( &mutex , NULL);

    //创建 5 个生产者线程，5个消费者线程
    pthread_t  ptids[5] , ctids[5];
    for( int i = 0 ; i < 5 ; i++){
        pthread_create( &ptids[i] , NULL  , producer , NULL);
        pthread_create( &ctids[i] , NULL  , customer , NULL);
        //设置线程分离
        pthread_detach(ptids[i]);
        pthread_detach(ctids[i]);
    }

    while(1){
        sleep(2);
    }
    //释放互斥量
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
    return 0;
}