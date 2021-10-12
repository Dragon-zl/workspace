#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{
    char z = *(const char *)0;
    printf("I read from location zero\n");
    exit(EXIT_SUCCESS);
}
/*
段错误 (核心已转储)
尝试直接从零地址处读取数据
这次在你和内核之间并没有GNU的libc库存在，于是，程序被终止了
要注意的是，有些版本的UNIX系统允许从零地址处读取数据，但Linux不允许
*/