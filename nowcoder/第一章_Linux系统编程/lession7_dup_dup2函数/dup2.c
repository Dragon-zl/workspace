#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
/*
    int dup2(int oldfd, int newfd);
    作用：重定向文件描述符
        例：oldfd指向 a.txt ， newfd 指向 b.txt 
            指向成功后：newfd 和 b.txt 做close ， newfd 指向了 a.txt
    oldfd必须是一个有效的文件描述符
    如果 oldfd 和 newfd 相同，调用该函数相对于什么也没做

    返回值：返回一个文件描述符，该文件描述符和newfd相同
*/
int main()
{
    int fd1 = open( "1.txt" , O_CREAT | O_RDWR , 0664);
    int fd2 = open( "2.txt" , O_CREAT | O_RDWR , 0664);

    int fd3 = dup2( fd1 , fd2 );

    char * str = "hello world";
    write( fd2 , str , strlen(str));

    printf("fd1 : %d , fd2 : %d , fd3 : %d\n" , fd1 , fd2 , fd3 );
    close(fd1);
    close(fd2);
    return 0;
}