/*
    SIGCHLD信号产生的条件

    1、子进程终止时
    2、子进程接收到 SIGSTOP 信号停止时
    3、子进程处在停止态，接受到 SIGCONT后唤醒时

    以上三种条件都会给父进程发送 SIGCHLD 信号，父进程默认会忽略该信号

    SIGCHLD信号可以解决僵尸进程问题
*/
/*
    SIGCHLD信号可以解决僵尸进程问题
*/
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
void  My_fun(int num){
    printf("捕捉到的信号  %d\n" , num);
    //回收子进程资源
    while(1){
        int ret = waitpid( -1 , NULL  , WNOHANG);
        if( ret > 0){
            printf("child die , pid = %d\n" , ret);
        }
        else if(ret == 0)//还有子进程活着
        {
            break;
        }
        else if( ret == -1 )
        {
            //没有子进程了
            break;
        }
    }
}
int main(){
    //提前设置好阻塞信号集，因为有可能子进程已经结束，父进程还没设置好捕捉
    sigset_t  set;
    sigemptyset( &set);
    sigaddset( &set , SIGCHLD);
    sigprocmask( SIG_BLOCK , &set , NULL);

    pid_t  pid;
    for( int i = 0 ; i < 20 ; i++){
        pid = fork();
        if( pid == 0){
            break;
        }
    }
    if( pid > 0){//父进程
        //捕捉子进程死亡时发送的 SIGCHLD 信号
        struct sigaction act;
        act.sa_flags = 0;
        act.sa_handler = My_fun;
        sigemptyset( &act.sa_mask);
        
        sigaction( SIGCHLD , &act , NULL);

        //注册完信号捕捉以后，解除阻塞
        sigprocmask( SIG_UNBLOCK , &set , NULL);
        
        sleep(5);

    }else if( pid == 0 ){//子进程
        printf("child process , pid = %d\n" , getpid());
    }
    return 0;
}