#include <stdio.h>
#include <unistd.h>
/*
    int execlp(const char *file, const char *arg, ...);

        execlp函数会到环境变量中查找指定的可执行文件，如果找到了就执行，找不到
        就执行失败

        参数：
            - file: 需要执行的可执行文件的文件名
            a.out
            ps
            - arg ：是可执行文件所需要的参数列表
                第一个参数一般是可执行程序的文件名，没什么用
                从第二个参数开始往后，就是程序执行所需要的参数列表。
                参数最后需要以 NULL 结束(哨兵)
            -返回值
                只有调用失败，才有返回值，返回-1，并且设置errno
*/
int main()
{
    //在子进程中调用 execl 函数
    pid_t  pid = fork();

    if( pid > 0){
        //父进程
        printf("I am parent process , pid = %d\n" , getpid());
        sleep(1);
    }
    else{
        //子进程
        execlp("ps" , "ps" , "aux" , NULL);
        printf("I am child process , pid = %d\n" , getpid());
    }
    for(int i = 0 ; i < 3 ; i++){
        printf("i = %d , pid = %d\n" , i , getpid());
    }
    return 0;
}