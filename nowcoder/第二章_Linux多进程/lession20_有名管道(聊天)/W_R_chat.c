#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(){

    //1、判断管道是否存在 ，不存在则创建，存在则不创建
    int ret = access( "fifo_1", F_OK);
    if(ret == -1){
        //管道不存在
        printf("fifo_1 不存在，创建\n");
        ret = mkfifo( "fifo_1", 0664 );
        if( -1 == ret ){
            perror("mkfifo:");
            exit(0);
        }
    }
    ret = access( "fifo_2", F_OK);
    if(ret == -1){
        //管道不存在
        printf("fifo_2 不存在，创建\n");
        ret = mkfifo( "fifo_2", 0664 );
        if( -1 == ret ){
            perror("mkfifo:");
            exit(0);
        }
    }
    //2、以只写的方式，打开 fifo1 , 只读的方式打开管道2
    int fdW = open( "fifo_1" , O_WRONLY );//只写
    if( -1 == fdW ){
        perror("open fifo_1:");
        exit(0);
    }
    printf("打开管道 fifo_1 ， 等待写入。。。\n");

    int fdR = open( "fifo_2" , O_RDONLY);//只读
    if( -1 == fdR ){
        perror("open fifo_2:");
        exit(0);
    }
    printf("打开管道 fifo_2 ， 等待读取。。。\n");
    //3、循环的写读数据
    char buf[128] = {0};
    while(1){
        bzero( buf , sizeof(buf));
        fgets( buf , sizeof(buf) , stdin);//获取键盘写入
        //写管道 fifo_1
        write( fdW , buf , strlen(buf));
        //读管道 fifo_2
        bzero( buf , sizeof(buf));
        ret = read( fdR , buf , sizeof(buf) - 1);
        if(ret <= 0){
            perror("read:");
            break;
        }
        printf("%s" , buf);
        //设置退出聊天协议
        if( strcmp(buf , "bye\n") == 0){
            break;
        }
    }
    close(fdR);
    close(fdW);
    return 0;
}