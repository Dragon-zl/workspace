/*
    #include <sys/mman.h>

    void *mmap(void *addr, size_t length, int prot, int flags,
                int fd, off_t offset);
    int munmap(void *addr, size_t length);

        
*/
/*
    内存映射通信，是非阻塞的
*/
/*
    内存映射不只可以用于进程间通信，也可以用于其他场景
    以下练习，拷贝文件
*/
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
int main(){
    int fd = open( "nowcode.txt" , O_RDONLY);
    if( -1 == fd ){
        perror("open:");
        exit(0);
    }
    int size = lseek( fd , 0 , SEEK_END);//获取被拷贝文件的长度
    int fdcp = open("copy.txt" , O_RDWR | O_CREAT , 0664);//创建一个新文件
    if( fdcp == -1){
        perror("open copy:");
        exit(0);
    }
    //拓展文件，使得新文件长度和被拷贝文件长度一直
    truncate( "copy.txt" , size);
    write(fdcp , " " , 1);
    //对被拷贝文件进行 映射
    char * ptr_src = (char *)mmap( NULL , size  , PROT_READ , MAP_SHARED , fd , 0);
    if(ptr_src == MAP_FAILED){
        perror("mmap fd:");
        exit(0);
    }
    //对新文件进行映射
    char * ptr_test = (char *)mmap( NULL , size  , PROT_READ | PROT_WRITE , MAP_SHARED , fdcp , 0);
    if(ptr_test == MAP_FAILED){
        perror("mmap fdcp:");
        exit(0);
    }
    strncpy( ptr_test , ptr_src , size);

    //回收映射
    munmap( (void *)ptr_test , size);
    munmap( (void *)ptr_src , size);
    //关闭文件
    close(fdcp);
    close(fd);
    return 0;
}