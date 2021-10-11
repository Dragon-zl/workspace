#include <stdlib.h>
#include <stdio.h>
#include <time.h>
int main()
{
    time_t  the_tm;
    (void)time(&the_tm);
    printf( "The time is %s\n" , ctime(&the_tm));
    exit(0);
}
/*
实验解析
ctime.c程序调用time函数得到底层时间值，
让ctime做所有的艰巨工作，把时间值转换成可读的字符串，然后打印它。
*/