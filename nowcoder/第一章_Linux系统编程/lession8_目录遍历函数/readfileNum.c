#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
/*
        DIR *opendir(const char *name);
        返回值：
            正确，返回一个 DIR * 的目录流
            错误，返回 NULL
        struct dirent *readdir(DIR *dirp);
        返回值：
            失败或者读取到目录末尾: 返回 NULL
            成功：
            struct dirent *  读取到的文件信息结构体指针
            struct dirent {
               ino_t          d_ino;        //此目录进入点的inode
               off_t          d_off;        目录文件开头到此目录进入点的偏移
               unsigned short d_reclen;     d_name的长度，不包含NULL字符
               unsigned char  d_type;       所指的文件类型  { DT_BLK —— 块设备   DT_CHR —— 字符设备   DT_DIR —— 目录 
                                                            DT_LNK —— 软连接    DT_FIFO —— 管道     DT_REG —— 普通文件
                                                            DT_SOCK —— 套接字   DT_UNKNOWN —— 未知 }
                                             
               char           d_name[256];  文件名 
           };

        int closedir(DIR *dirp);
*/
//获取 某目录下 所有普通文件的个数
int GetFileNum( const char * path);

int main(int argc , char * argv[])
{
    if( argc < 2)
    {
        printf("%s  DirPath\n");
        return -1;
    }
    
    printf ("普通文件的个数：%d\n" , GetFileNum( argv[1] ));
    return 0;
}

//获取 某目录下 所有普通文件的个数
int GetFileNum( const char * path){
    if( path == NULL)
    {   //判断输入的参数格式是否正确
        return 0;
    }
    int  Num = 0;
    //1、打开目录
    DIR * my_DIR = opendir( path );
    if( my_DIR == NULL){
        perror("opendir:");
        return -1;  //如果打开失败
    }
    //2、循环读取该目录，并判断是否为普通文件 或 目录
    struct dirent * my_dirent;
    while(1)
    {
        my_dirent = readdir( my_DIR );
        if( NULL == my_dirent)
        {   //即 读取到了文件末尾
            break;
        }
        //获取文件 名称 
        char  * file_name  = my_dirent->d_name;
        //忽略掉 .和.. 目录
        if( strcmp(file_name , ".") == 0 || strcmp(file_name , "..") == 0 )
        {
            continue;
        }

        if( my_dirent->d_type == DT_REG)
        {   // 即 为普通文件
            Num++;
        }
        else if( my_dirent->d_type == DT_DIR )
        {   // 如果读取到目录
            char  new_dir_path[256];
            sprintf( new_dir_path , "%s/%s" , path , file_name);
            //拼接成 新的目录 path
            // 递归
            Num += GetFileNum( new_dir_path );
        }
    }
    closedir(my_DIR);
    return Num;
}