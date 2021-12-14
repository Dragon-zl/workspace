/*
    信号量的类型：
        sem_t
    #include <semaphore.h>
    int sem_init(sem_t *sem, int pshared, unsigned int value);
        功能：初始化信号量
        参数：
            sem：信号量变量的地址
            pshared：
                - 0 用在线程间
                非 0 用在进程间
            value：信号量中的值
    int sem_destroy(sem_t *sem);
        功能：释放信号量
    int sem_wait(sem_t *sem);
        功能：对信号量加锁，信号量的值 -1 ， 如果值为 0 ，就阻塞
    int sem_trywait(sem_t *sem);
    int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

    int sem_post(sem_t *sem);
        功能：
            对信号量解锁，信号量值加一

    int sem_getvalue(sem_t *sem, int *sval);
*/
/*
    案例：
        生产者、消费者模型(粗略的版本)
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
//互斥量
pthread_mutex_t  mutex;
//信号量
sem_t  psem;

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

        int sval;
        sem_getvalue( &psem , &sval );
        if( sval < 10 ){    //容器最大容量为 10 ， 最多存在 10个未执行的任务
            struct  Node *  node = (struct  Node * )malloc(sizeof(struct  Node));
            node->num = rand() % 1000;  //取随机值
            node -> next = head;
            head = node;    //头插
            printf("add node, num: %d, tid: %ld\n" , node->num , pthread_self());

            sem_post(&psem);//信号量的值 +1
        }
        //解锁
        pthread_mutex_unlock( &mutex );
        usleep(100);
    }
    return NULL;
}
//消费者函数
void * customer( void * arg)
{
    //头删法
    while(1)
    {
        //加锁
        pthread_mutex_lock( &mutex );
        int sval;
        sem_getvalue(&psem , &sval);
        if( sval > 0 )//必须 ＞ 0 ， 代表 有任务
        {
            //先保存头节点
            struct Node * node = head;
            head = head -> next;
            printf("del node, num: %d, tid: %ld\n" , node->num , pthread_self());
            free(node);
            sem_wait(&psem);//信号量的值 -1
        }
        //解锁
        pthread_mutex_unlock( &mutex );
        usleep(100);
    }
    return NULL;
}
int main()
{
    //初始化互斥量
    pthread_mutex_init( &mutex , NULL);
    //初始化信号量
    sem_init( &psem , 0 , 0);//生产者的信号量的值为 0 



    //创建 5 个生产者线程，5个消费者线程
    pthread_t  ptids[5] , ctids[5];
    for( int i = 0 ; i < 5 ; i++)
    {
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
    //释放信号量
    sem_destroy(&psem);
    pthread_exit(NULL);
    return 0;
}