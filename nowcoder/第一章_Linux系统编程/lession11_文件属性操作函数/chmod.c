#include <sys/stat.h>
#include <stdio.h>
//  int chmod(const char *pathname, mode_t mode);
/*
        参数：mode
            需要修改的权限值，八进制的数
*/
int main()
{
    //例如当前 a.txt
//  -rw-rw-r--. 1 dzl dzl     0 11月 23 14:44 a.txt  
    //将权限修改为 0775
    int ret = chmod("a.txt" , 0775);
    if( ret == -1)
    {
        perror("chmod:");
        return -1;
    }
    //修改后 使用 ls -l 查看
//  -rwxrwxr-x. 1 dzl dzl     0 11月 23 14:44 a.txt
    return 0;
}