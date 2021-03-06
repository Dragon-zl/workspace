#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

const char * test_file =  "./test_lock";

#define SIZE_TO_TRY  5

void  show_lock_info(struct flock * to_show);
int main()
{
    int file_desc;
    int res;
    struct  flock  region_to_test;
    int start_byte;
    //打开一个文件描述符
    file_desc = open( test_file , O_RDWR | O_CREAT , 0666 );
    if( !file_desc )
    {
        fprintf( stderr , "Unable to open %s for read/write\n" , test_file);
        exit(EXIT_FAILURE);
    }
    for( start_byte = 0; start_byte < 99 ; start_byte += SIZE_TO_TRY )
    {
        //设置希望测试的文件区域
        region_to_test.l_type = F_WRLCK;
        region_to_test.l_whence = SEEK_SET;
        region_to_test.l_start = start_byte;
        region_to_test.l_len = SIZE_TO_TRY;
        region_to_test.l_pid = -1;          //持有锁的进程的标识符
        printf("Testing  F_WRLCK  on region from %d to %d\n" , start_byte , start_byte + SIZE_TO_TRY);

        //现在测试文件上的锁
        res = fcntl( file_desc , F_GETLK , &region_to_test);  
        //F_GETLK:用于获取fildes（第一个参数）打开的文件的锁信息。它不会尝试去锁定文件
        if( res == -1 )
        {
            fprintf(stderr , "F_GETLK failed\n");
            exit(EXIT_FAILURE);
        }
        if( region_to_test.l_pid != -1 )    //即该文件区域有对其上锁的进程 
        {
            printf("Lock would fail , F_GETLK return:\n");
            show_lock_info(&region_to_test);
        }
        else
        {
            printf("F_WRLCK - Lock would succeed\n");
        }

        //用共享锁重复测试一次
        region_to_test.l_type = F_RDLCK;
        region_to_test.l_whence = SEEK_SET;
        region_to_test.l_start = start_byte;
        region_to_test.l_len = SIZE_TO_TRY;
        region_to_test.l_pid = -1;
        printf("Testing  F_RDLCK  on region from %d to %d\n" , start_byte , start_byte + SIZE_TO_TRY);
        res = fcntl( file_desc , F_GETLK , &region_to_test);  
        if( res == -1 )
        {
            fprintf(stderr , "F_GETLK failed\n");
            exit(EXIT_FAILURE);
        }
        if( region_to_test.l_pid != -1 )
        {
            printf("Lock would fail , F_GETLK return:\n");
            show_lock_info(&region_to_test);
        }
        else
        {
            printf("F_RDLCK - Lock would succeed\n");
        }
    }
    close(file_desc);
    exit(EXIT_SUCCESS);
}
void show_lock_info(struct flock * to_show)
{
    printf("\tl_type %d , " , to_show->l_type);     //l_type的值为0对应的定义为F_RDLCK,  l_type的值为1表明锁失败的原因是已经存在一个写锁了
    printf("l_whence %d , " , to_show->l_whence);
    printf("l_start %d , " , (int)to_show->l_start);
    printf("l_len %d , " , (int)to_show->l_len);
    printf("l_pid %d\n" , to_show->l_pid);
}