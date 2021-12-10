/*
    #include <sys/types.h>
    #include <sys/stat.h>

    int mkfifo(const char *pathname, mode_t mode);
        参数：
            pathname：管道名称的路径
            mode：文件的权限，和open的mode是一样的
        返回值：
            成功：返回0
            失败：返回-1
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    //判断文件是否存在
    int ret = access( "fifo" , F_OK);
    if( -1 == ret ){
        //管道不存在
        printf("管道不存在 ， 创建管道\n");
        ret = mkfifo( "fifo_1" , 0664);
        if( -1 == ret ){
            perror( "mkfifo:");
            exit(0);
        }
    }
    
    return 0;
}