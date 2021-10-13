#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

const char * lock_file = "./LCK.test";
int main()
{
    int file_desc;
    int tries = 10;
    while(tries--)
    {
        file_desc = open(lock_file , O_RDWR|O_CREAT|O_EXCL , 0444);
        if( file_desc == -1 )
        {
            printf("%d - Lock already present\n" , getpid());
            sleep(3);
        }
        else
        {
            printf("%d - I have exclusive access\n" , getpid());
            sleep(1);
            (void)close(file_desc);
            (void)unlink(lock_file);        //删除锁文件来释放锁
            sleep(2);
        }
    }
    exit(EXIT_SUCCESS);
}