

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(){

    //1、判断文件是否存在
    int ret = access( "fifo" , F_OK);
    if( -1 == ret ){
        //管道不存在
        printf("管道不存在 ， 创建管道\n");
        ret = mkfifo( "fifo" , 0664);
        if( -1 == ret ){
            perror( "mkfifo:");
            exit(0);
        }
    }
    //2、打开管道
    int fd = open( "./fifo" , O_WRONLY);//以只写的方式打开
    if( -1 == fd ){
        perror("open:");
        exit(0);
    }
    //3、往管道中写数据
    for(int i = 0 ; i < 10 ; i++){
        char buf[1024];
        sprintf( buf , "hello %d\n" , i);
        printf("wirt : %d\n" , i);
        write( fd , buf , strlen(buf));
    }
    //4、关闭管道文件
    close(fd);
    return 0;
}