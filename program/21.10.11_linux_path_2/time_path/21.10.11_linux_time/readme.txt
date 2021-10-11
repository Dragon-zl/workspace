
#include <time.h>

double  difftime( time_t  time1 , time_t  time2);

difftime函数计算两个时间值之间的差，并将time1-time2的值作为浮点数返回。
对Linux来说，time函数的返回值是一个易于处理的秒数，但考虑到最大限度的可移植性，你最好使用difftime。


