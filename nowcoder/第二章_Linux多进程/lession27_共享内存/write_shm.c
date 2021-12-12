/*
    案例：创建共享内存
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    //1、创建共享内存
    int shmid = shmget( 0x16 , 4096, IPC_CREAT|0664);
    printf("shmid = %d\n" , shmid);
    if( shmid == -1){
        perror("shmget:");
        exit(0);
    }
    //2、和当前进程关联
    void * ptr = shmat( shmid , NULL , 0);//可读可写
    //3、写数据
    char * str = "hello world";
    memcpy( ptr , str , strlen(str));
    printf("按任意键继续\n");
    getchar();
    //4、解除关联
    shmdt(ptr);
    //5、删除共享内存
    shmctl( shmid , IPC_RMID , NULL);
    return 0;

}