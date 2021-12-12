#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
/*
       int truncate(const char *path, off_t length);
        作用：缩减或者扩展 指定文件的 大小
                缩减会从文件末尾开始裁剪，扩展会从文件末尾添加空字符
        参数：length
                最终的文件大小
*/

int main()
{
    int ret = truncate("a.txt" , 20);
    if( -1 == ret )
    {
        perror("truncate");
        return -1;
    }
    return 0;
}