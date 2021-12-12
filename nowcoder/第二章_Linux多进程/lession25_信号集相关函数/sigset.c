/*
    #include <signal.h>

    以下函数都是对用户自定义的信号集进行修改：

    int sigemptyset(sigset_t *set);
        功能：清空信号集中的数据，将信号集中的所有标志位都置为 0
        参数：set ， 传出参数，需要操作的信号集
        返回值：成功返回 0 ，失败返回 -1

    int sigfillset(sigset_t *set);
        功能：填充信号集，将信号集中的所有标志位都置为 1
        参数：set ， 传出参数，需要操作的信号集
        返回值：成功返回 0 ，失败返回 -1
    int sigaddset(sigset_t *set, int signum);
        功能：设置信号集中的某一个信号的标志位为 1
        参数：
            set ： 传出参数，需要操作的信号集
            signum：需要设置的那个信号

        返回值：成功返回 0 ，失败返回 -1
    int sigdelset(sigset_t *set, int signum);
        功能：设置信号集中的某一个信号的标志位为 0
        参数：
            set ： 传出参数，需要操作的信号集
            signum：需要设置的那个信号

        返回值：成功返回 0 ，失败返回 -1
    int sigismember(const sigset_t *set, int signum);
        功能：判断某个信号是否阻塞
        参数：
            set ：  需要操作的信号集
            signum：需要判断的那个信号

        返回值：
            1：signum信号被阻塞
            0：signum信号不阻塞
            -1：失败
*/

#include <signal.h>
#include <stdio.h>  
int main(){
    //1、创建一个信号集
    sigset_t  set;
    //2、清空信号集
    sigemptyset( &set );
    //3、判断SIGINT在信号集里是阻塞还是非阻塞
    int ret = sigismember( &set, SIGINT);
    if( 0 == ret ){
        printf("SIGINT  非阻塞\n");
    }
    else if( 1 == ret){
        printf("SIGINT  阻塞\n");
    }
    //4、设置两个信号的标志位 为 1
    printf("设置 SIGINT SIGQUIT 两个信号的标志位为 1 ，即阻塞\n");
    sigaddset( &set, SIGINT);
    sigaddset( &set, SIGQUIT);
    //5、查询是否设置成功
    ret = sigismember( &set, SIGINT);
    if( 0 == ret ){
        printf("SIGINT  非阻塞\n");
    }
    else if( 1 == ret){
        printf("SIGINT  阻塞\n");
    }
    ret = sigismember( &set, SIGQUIT);
    if( 0 == ret ){
        printf("SIGQUIT  非阻塞\n");
    }
    else if( 1 == ret){
        printf("SIGQUIT  阻塞\n");
    }
    //6、清空某个信号的标志位
    sigdelset( &set, SIGQUIT);
    printf("清空信号SIGQUIT的标志位\n");
    //7、再查询标志位
    ret = sigismember( &set, SIGQUIT);
    if( 0 == ret ){
        printf("SIGQUIT  非阻塞\n");
    }
    else if( 1 == ret){
        printf("SIGQUIT  阻塞\n");
    }
    return 0;
}