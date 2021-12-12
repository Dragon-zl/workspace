#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
/*
    int dup(int oldfd);
    作用：复制一个文件描述符，并返回
        它会从空闲的文件描述符表中，选一个最小文件描述符作为新的的文件描述符
    
    
*/
       
int main()
{
    int fd = open( "a.txt" , O_RDWR | O_CREAT , 0664);
    int fd1 = dup(fd);
    if( fd1 == -1 )
    {
        perror("dup:");
        return -1;
    }
    printf("fd: %d , fd1: %d\n" , fd , fd1);
    close(fd);
    char * str = "hello world";
    write( fd1 , str , strlen(str));
    close(fd1);
    return 0;
}