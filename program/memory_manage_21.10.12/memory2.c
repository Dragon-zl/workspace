/*
请求全部的物理内存
在程序memory2.c中，我们将请求比机器物理内存容量更多的内存。
你需要根据机器的具体情况来调整宏定义PHY_MEM_MEGS：
*/
#include <stdlib.h>
#include <stdio.h>
#define  A_MEGABYTE (1024 * 1024)
#define  PHY_MEM_MEGS  4096

int main()
{
    char * some_memory;
    size_t  size_to_allocate = A_MEGABYTE;
    int megs_obtained = 0;
    while( megs_obtained < (PHY_MEM_MEGS * 2))
    {
        some_memory = (char *)malloc(size_to_allocate);
        if( some_memory != NULL)
        {
            megs_obtained ++;
            sprintf(some_memory , "Hello world\n");   
            printf( "%s - now allocated %d Megabyytes\n" , some_memory , megs_obtained);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}