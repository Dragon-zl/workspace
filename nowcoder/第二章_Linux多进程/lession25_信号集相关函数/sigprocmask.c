/*
    #include <signal.h>
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
        功能：将自定义信号集中的数据，设置到内核中(设置阻塞 | 解除阻塞 | 替换)
        参数：
        how：（如何对内核信号集进行处理）
            SIG_BLOCK ：将用户设置的信号集添加到内核中，相当于与 内核信号集做 | 或运算
                        mask(内核信号集) | set
            SIG_UNBLOCK：根据用户设置的信号集数据，清除内核信号集的标志位
                        mask &= ~set
            SIG_SETMASK ：覆盖掉内核中的信号集
        set：
            已经初始化好的用户定义的信号集
        oldset：
            保存的设置之前的内核信号集的状态，可以是NULL
        返回值：
            成功 0
            失败 -1
                设置错误号：EFAULT、EINVAL

    int sigpending(sigset_t *set);
        功能：获取内核中的未决信号集
        参数：
            set  传出参数，保存内核中的未决信号集的信息
        返回值：
            成功 0
            失败 -1
*/
/*
    案例：
        编写一个程序，把所有的常规信号（1-31）的未决状态打印到终端
        （循环打印）
        设置某些信号阻塞，使用键盘产生这些信号
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int  main(){
    //1、设置 2 3 号信号为阻塞
    //创建一个信号集
    sigset_t  set;
    //清空信号集
    sigemptyset( &set );
    //设置两个信号的标志位 为 1
    sigaddset( &set, SIGINT);
    sigaddset( &set, SIGQUIT);

    //2、修改内核中的信号集
    sigprocmask(SIG_BLOCK , &set , NULL);
    int num = 0;
    //3、获取当前未决信号集的信息
    while(1){
        sigset_t  pendingset;
        sigemptyset( &pendingset );
        sigpending( &pendingset );

        //遍历获取到的信号集信息的前 32 位
        for( int i = 1 ; i <= 31; i++){
            if( sigismember( &pendingset, i) == 1 ){
                printf("1");
            }else if( sigismember( &pendingset, i) == 0 ){
                printf("0");
            }else{
                perror("sigismember");
                exit(0);
            }

        }
        printf("\n");
        sleep(1);
        num++;
        if( 10 == num ){    //解除设置的信号阻塞
            sigprocmask(SIG_UNBLOCK , &set , NULL);
        }
    }
    return 0;
}