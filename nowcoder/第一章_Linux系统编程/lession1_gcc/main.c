#include <stdio.h>
#define  PI  3.14
int main()
{
    //这是测试代码
    int sum = PI + 10;

#ifdef  DEBUG
    printf("我是一个程序猿 ， 我不会爬树\n");
#endif
    return 0;
}