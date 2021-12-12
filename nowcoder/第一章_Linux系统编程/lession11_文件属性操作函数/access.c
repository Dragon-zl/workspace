#include <stdio.h>
#include <unistd.h>

// int access(const char *pathname, int mode);
/*
        参数：mode
                R_OK：判断是否有读权限
                W_OK：判断是否有写权限
                X_OK：判断是否有执行权限
                F_OK：判断文件是否存在

                这里的权限拥有者指的是 当前程序进程

*/
int main()
{
    int ret  = access("a.txt" , F_OK);
    if( ret == -1)
    {
        perror("access:");
        return -1;
    }
    printf("文件 a.txt 存在\n");
    return 0;
}