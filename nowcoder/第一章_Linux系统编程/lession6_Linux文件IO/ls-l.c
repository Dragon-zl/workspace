#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
/*
    模拟实现 ls -l 功能
    -rwxrwxr-x 1 dzl dzl 17584 十一月 10 20:49 main
*/
int main(int argc , char * argv[])
{
    if( argc < 2 )//即 用户没用输入文件名参数
    {
        printf("%s  filename" , argv[0]);
        exit(EXIT_FAILURE);
    }
    struct stat file_stat;
    int st = stat(argv[1] , &file_stat);
    //stat文件信息
    if( st == -1)
    {
        perror("stat:");
    }
    //1、获取文件类型、文件权限
    char auth_array[11] = {0};
    switch(file_stat.st_mode & S_IFMT)
    {
        case S_IFSOCK://套接字
            auth_array[0] = 's';
            break;
        case S_IFLNK://符号链接(软链接)
            auth_array[0] = 'l';
            break;
        case S_IFREG://普通文件
            auth_array[0] = '-';
            break;
        case S_IFBLK://块设备
            auth_array[0] = 'b';
            break;
        case S_IFDIR://目录
            auth_array[0] = 'd';
            break;
        case S_IFCHR://字符设备
            auth_array[0] = 'c';
            break;
        case S_IFIFO://管道
            auth_array[0] = 'f';
            break;
        case S_IFMT://掩码
            auth_array[0] = 'm';
            break;
        default:
            auth_array[0] = '?';
    }
    //判断文件的访问权限
    int i = 0;
    auth_array[++i] = file_stat.st_mode & S_IRUSR ?  'r':'-';
    auth_array[++i] = file_stat.st_mode & S_IWUSR ?  'w':'-';
    auth_array[++i] = file_stat.st_mode & S_IXUSR ?  'x':'-';

    auth_array[++i] = file_stat.st_mode & S_IRGRP ?  'r':'-';
    auth_array[++i] = file_stat.st_mode & S_IWGRP ?  'w':'-';
    auth_array[++i] = file_stat.st_mode & S_IXGRP ?  'x':'-';

    auth_array[++i] = file_stat.st_mode & S_IROTH ?  'r':'-';
    auth_array[++i] = file_stat.st_mode & S_IWOTH ?  'w':'-';
    auth_array[++i] = file_stat.st_mode & S_IXOTH ?  'x':'-';

    //获取硬链接数
    int  linknum = file_stat.st_nlink;
    //文件所有者
    char * fileUser = getpwuid( file_stat.st_uid )->pw_name ;
    //文件所在组
    char * fileGrp = getgrgid( file_stat.st_gid )->gr_name ;
    //文件大小
    long int filesize = file_stat.st_size;
    //获取修改的时间
    char * time = ctime( (const time_t *)&file_stat.st_mtim );

    char mtime[512];
    strncpy( mtime , time , strlen(time) - 1 );

    //-rwxrwxr-x 1 dzl dzl 17584 十一月 10 20:49 main

    char buf[1024] = {0};
    sprintf( buf , "%s %d %s %s %ld %s %s" , auth_array ,linknum , fileUser , fileGrp , filesize ,mtime ,  argv[1]);

    printf("%s\n" , buf);
    exit(EXIT_SUCCESS);
}