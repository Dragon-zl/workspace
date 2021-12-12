/*
    案例：创建共享内存
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    //1、获取共享内存
    int shmid = shmget( 0x16 , 0, IPC_CREAT );
    printf("shmid = %d\n" , shmid);
    if( shmid == -1){
        perror("shmget:");
        exit(0);
    }
    //2、和当前进程关联
    void * ptr = shmat( shmid , NULL , 0);//可读可写
    //3、读数据
    printf("%s\n"  , (char *)ptr);
    printf("按任意键继续\n");
    getchar();
    //4、解除关联
    shmdt(ptr);
    //5、删除共享内存
    shmctl( shmid , IPC_RMID , NULL);
    return 0;

}