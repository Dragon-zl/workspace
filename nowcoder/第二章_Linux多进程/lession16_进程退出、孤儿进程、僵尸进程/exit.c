#include <stdio.h>

/*
    #include <stdlib.h>
    void  exit( int status );

    #include <unistd.h>
    void _exit( int status );

    参数：
        status 为进程退出的状态，父进程可以接受子进程退出时的状态
*/
#include <stdlib.h>
#include <unistd.h>
int main()
{
    printf("hello\n");
    printf("world");

    //_exit(0);//终端 不会 打印 world ， 因为没有刷新IO缓冲区，world还在缓冲区中
    exit(0);//终端会打印 world
}