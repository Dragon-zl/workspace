#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main()
{
    int fd = open("open.c" , O_RDWR | O_CREAT , 0777 );
    //  第三个参数 指定 被创建文件的权限  实际上结构是 0775
    if( -1 == fd)
    {
        perror("open:");
    }
    close(fd);
    return 0;
}