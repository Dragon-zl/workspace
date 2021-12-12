#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
/*
    int fcntl(int fd, int cmd, ...  );
        参数：
            fd：表示要操作的那个文件描述符
            cmd：表示对文件描述符如何操作
                F_DUPFD ： 复制文件描述符 fd ，并一个新的文件描述符
                    int fd_new = fcntl( fd , F_DUPFD);
                F_GETFL: 获取文件的状态flag
                F_SETFL：设置文件的状态flag
                    必选项：O_RDONLY, O_WRONLY, O_RDWR
                    可选性：O_APPEND   O_NONBLOCK
                            O_APPEND 追加数据
                            O_NONBLOCK  设置成 非阻塞
*/          
#include <string.h>
int main()
{
    //1、复制文件描述符
    //2、修改或者获取文件状态flag
    int fd = open("1.txt" , O_RDWR );

    // 获取文件描述符状态flag
    int flag = fcntl( fd , F_GETFL);
    flag |= O_APPEND;
    // 修改文件描述符状态的flag ， 给flag加入 O_APPEND这个标记
    int ret = fcntl( fd , F_SETFL , flag);

    char * str = "hello";
    write( fd , str , strlen(str));
    close(fd);
    return 0;
}