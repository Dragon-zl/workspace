#include <time.h>
#include <stdio.h>
#include <stdlib.h>
int main()
{
    time_t  now_time;
    struct  tm  *  the_tm;
    (void)time(&now_time);

    //the_tm = gmtime( &now_time );
    the_tm = localtime( &now_time );

    printf( "Raw time is ld\n" , now_time);
    printf( "gmtime gives:\n");
    printf( "date: %2d/%2d/%2d\n",
            the_tm->tm_year + 1900, the_tm->tm_mon + 1 , the_tm->tm_mday);
    printf( "time: %02d:%02d:%02d\n",
            the_tm->tm_hour , the_tm->tm_min , the_tm->tm_sec);
    exit(0);
}
/*
要看当地时间，你需要使用localtime函数。

已分解出来的tm结构再转换为原始的time_t时间值，你可以使用mktime函数

time_t  mktime(struct  tm * timeptr)
如果tm结构不能被表示为time_t值，mktime将返回-1。

*/