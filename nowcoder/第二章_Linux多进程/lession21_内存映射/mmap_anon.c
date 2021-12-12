/*
    匿名映射：
        不需要文件实体进程的一个内存映射 , 
        在mmap 时，参数fd 为 -1 ， prot 需要添加MAP_ANONYMOUS ， 匿名标志


    void *mmap(void *addr, size_t length, int prot, int flags,
                int fd, off_t offset);
    int munmap(void *addr, size_t length);

*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>
int main(){

    //1、创建匿名映射区
    int len = 4096;
    void  * ptr =  mmap( NULL , len , PROT_READ | PROT_WRITE , MAP_SHARED | MAP_ANONYMOUS , -1 , 0);
    if( ptr == MAP_FAILED){
        perror("mmap:");
        exit(0);
    }

    //父子进程间通信
    pid_t  pid = fork();
    if( pid > 0){
        wait(NULL);
        printf("%s\n" , (char *)ptr);
    }
    else if( pid == 0){
        strcpy( ptr , "hello world");
    }
    //释放掉映射
    munmap(ptr , len);

    return 0;
}