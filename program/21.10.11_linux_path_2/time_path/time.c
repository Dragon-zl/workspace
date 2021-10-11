#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
int main()
{
    time_t  the_time;
    int i;
    for(i = 0 ; i < 10 ; i++)
    {
        the_time = time( ( time_t *)0 );
        fprintf( stdout , "The time is %ld\n" , the_time);
        sleep(2);
    }
    exit(0);
}
/*
实验解析
这个程序用一个空指针参数调用time函数，返回以秒数计算的时间和日期。
程序休眠两秒后再重复调用time函数，总共调用10次。
以从1970年开始计算的秒数来表示时间和日期，
对测算某些事情持续的时间是很有用的。
你可以把它考虑为简单地把两次调用time得到的值相减。
然而ISO/ANSIC标准委员会经过审议，并没有规定用time_t类型来测量任意时间之间的秒数，
他们发明了一个函数difftime，该函数用来计算两个time_t值之间的秒数并以double类型返回它。
*/