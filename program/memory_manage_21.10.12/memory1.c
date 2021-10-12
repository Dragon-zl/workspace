#include <stdlib.h>
#include <stdio.h>

#define A_MEGABYTE  (1024 * 1024)
int main()
{
    char * some_memory;
    int megabyte = A_MEGABYTE;
    int exit_code = EXIT_FAILURE;       //C语言中 宏EXIT_FAILURE和EXIT_SUCCESS定义在头文件stdlib.h中,是一个符号常量，定义如下
                                        /*
                                        #define EXIT_FAILURE 1
                                        #define EXIT_SUCCESS 0
                                        */

    some_memory = (char *)malloc(megabyte);
    //malloc函数给它返回一个指向1MB内存空间的指针
    //malloc函数可以保证其返回的内存是地址对齐的
    if( some_memory != NULL)    //检查并确保malloc函数被成功调用
    {
        sprintf(some_memory , "Hello world\n");     
        fprintf( stdout , "%s" , some_memory);
        exit_code = EXIT_SUCCESS;
    }
    exit(exit_code);
}
/*
    32Linux系统可寻址的地址空间可达4GB    : 32位 =2^ 32 B = 4 * 2^30B = 4GB
    64位的linux采用4级页表，支持的最大物理内存为64T  : 64位架构下，地址线是46个，所以最大的物理地址是2^46B，折合64TB
*/