/*
    #include <sys/types.h>
    #include <sys/wait.h>

    pid_t waitpid(pid_t pid, int *wstatus, int options);
        功能：回收指定进程号的子进程，可以设置为不阻塞
        参数：
            pid:
               pid > 0 子进程的 pid
               pid = 0 回收当前进程组的所有子进程
               pid = -1 表示回收所有的子进程 ， 相当于调用 wait()。 （最常用）
               pid < -1 某个进程组的组id的绝对值的相反数，回收指定进程组中的子进程
            options: 设置阻塞或者非阻塞
                0 : 阻塞
                WNOHANG ：非阻塞
        返回值：
            >0 : 返回子进程的id
            = 0 ：options = WNOHANG ，表示还有子进程活着
            -1 ：错误，或者没有子进程了
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main(){
    //有一个进程 , 创建 5 个子进程(兄弟)
    pid_t pid;
    for( int i = 0 ; i < 5 ; i++ )
    {
        pid = fork();
        if( pid == 0 ){
            break;
        }
    }
    if( pid > 0){
        while(1){
            printf("parent , pid = %d\n" , getpid());
            int st;
            //int ret = waitpid( -1 , &st , 0 );//如果参数 填 NULL， 即不需要保存子进程结束的状态信息
            int ret = waitpid( -1 , &st , WNOHANG );//非阻塞
            if( -1 == ret ){
                break;  //所以子进程都结束了
            }
            else if( 0 == ret ){
                //表名还有子进程活着
                continue;
            }
            else if( ret > 0 ){
                //回收到了子进程
                if( WIFEXITED(st) ){
                printf("子进程结束的状态吗 = %d\n" , WEXITSTATUS(st) );
                }
                if( WIFSIGNALED(st) ){
                printf("子进程被干掉的信号 = %d\n" , WTERMSIG(st) );
                }
            }
            printf("process die , pid = %d\n" , ret);
        }
    }
    else if( pid == 0 ){
        printf("child , pid = %d\n" , getpid());
        sleep(1);
        exit(0);//正常退出
    }
    return 0;
}
