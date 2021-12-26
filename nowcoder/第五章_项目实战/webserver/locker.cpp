#include "locker.h"



//构造函数
locker :: locker(){
    //初始化
    if( pthread_mutex_init( &My_mutex , NULL) != 0 ){
        throw  std :: exception();  //抛出异常对象
    }
}
//析构函数
locker :: ~locker(){
    pthread_mutex_destroy( &My_mutex );
}
//上锁
bool locker :: lock(){
    return pthread_mutex_lock( &My_mutex ) == 0;
}
//解锁
bool locker :: unlock(){
    return pthread_mutex_unlock( &My_mutex ) == 0;
}
//获取锁状态
pthread_mutex_t * locker :: get(){
    return &My_mutex;
}
//----------------------------------------------------
cond :: cond(){
    if( pthread_cond_init( &My_cond , NULL) != 0 ){
        throw  std :: exception();  //抛出异常对象
    }
}
cond :: ~cond(){
    pthread_cond_destroy(&My_cond);
}
bool cond :: wait(pthread_mutex_t * mutex){
    return pthread_cond_wait( &My_cond , mutex ) == 0;
}
bool cond :: time_wait(pthread_mutex_t * mutex , struct timespec  time){
    return pthread_cond_timedwait( &My_cond , mutex , &time) == 0;
}
//唤醒一个条件变量
bool cond :: signal_wait(){
    return pthread_cond_signal( &My_cond ) == 0;
}
//唤醒所有条件变量
bool cond :: broadcast(){
    return pthread_cond_broadcast(&My_cond);
}
//------------------------------------------------------------------------------
sem :: sem(){
    if( sem_init( &My_sem , 0 , 0) != 0){
        throw  std :: exception();  //抛出异常对象
    }
}
sem :: sem(int num){
    if( sem_init( &My_sem , 0 , num) != 0){
        throw  std :: exception();  //抛出异常对象
    }
}
sem :: ~sem(){
    sem_destroy(&My_sem);
}
//等待信号量
bool  sem :: wait(){
    return sem_wait(&My_sem) == 0;
}
//增加信号量
bool  sem :: post(){
    return sem_post(&My_sem) == 0;
}
//---------------------------------