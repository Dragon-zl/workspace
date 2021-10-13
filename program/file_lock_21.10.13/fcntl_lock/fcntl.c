
/*
为了试验锁定，你需要两个程序：一个用来锁定而另外一个进行测试。
第一个程序完成锁定。
*/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
const char * test_file = "./test_lock";
int main()
{
    int file_desc;
    int byte_count;
    char * byte_to_write = "A";
    struct  flock  region_1;
    struct  flock  region_2;
    int res;
    //打开(创建)文件
    file_desc = open( test_file ,O_RDWR | O_CREAT , 0666);
    if( !file_desc )
    {
        fprintf( stderr , "Unable to open %s for read/write\n" , test_file);
        exit(EXIT_FAILURE);
    }
    //写入数据
    for( byte_count = 0 ; byte_count < 100 ; byte_count++ )
    {
        (void)write(file_desc , byte_to_write , 1);
    }
    //把文件的10~30字节设为区域 1 ， 并在其上设置共享锁
    region_1.l_type = F_RDLCK;      //共享锁
    region_1.l_whence = SEEK_SET;   //文件头
    region_1.l_start = 10;          //该区域的第一个字节
    region_1.l_len = 20;
    //l_whence通常被设为SEEK_SET，这时l_start就从文件的开始计算。l_len参数定义了该区域的字节数。

    //把文件的40~50字节设为区域 2 ， 并在其上设置独占锁
    region_2.l_type = F_WRLCK;      
    region_2.l_whence = SEEK_SET;   
    region_2.l_start = 40;         
    region_2.l_len = 10;

    //现在锁定文件
    printf("Process %d locking file\n" , getpid());
    res = fcntl( file_desc , F_SETLK , &region_1);      //F_SETLK :试图对fildes指向的文件的某个区域加锁或解锁
    if( res == -1 )
    {
        fprintf(stderr , "Failed to lock region 1\n");
    }
    res = fcntl( file_desc , F_SETLK , &region_2);
    if( res == -1 )
    {
        fprintf(stderr , "Failed to lock region 2\n");
    }

    //然后等一会
    sleep(60);
    printf("Process %d closing file\n" , getpid());
    close(file_desc);
    exit(EXIT_SUCCESS);
}


