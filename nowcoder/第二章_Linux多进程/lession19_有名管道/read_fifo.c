#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(){
    //1、打开管道
    int fd = open( "./fifo" , O_RDONLY);//只读
    if( -1 == fd ){
        perror("open:");
        exit(0);
    }
    //2、读数据
    while(1){
        char buf[1024] = {0};
        int len = read( fd , buf , sizeof(buf));
        if( len == 0 ){
            printf("写端断开连接了。。。\n");
            break;
        }
        printf("recv buf : %s\n" , buf);
    }
    //3、关闭管道
    close(fd);
    exit(0);
}