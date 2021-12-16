#include <stdio.h>
#include <arpa/inet.h>
/*
    案例：
        使用 字节序转换函数
*/
int main(){
    unsigned  short a = 0x0102;
    unsigned  short b = htons( a );//小端转大端

    //htons  用来转换 端口
    printf("a : %x\n" , a);//使用 16进制 形式打印a
    printf("b : %x\n" , b);//使用 16进制 形式打印b

    printf("====================\n");
    //htonl  用来转换 IP
    char  buf[4] = {192 , 168 , 1 , 100};
    int num = *(int *)buf;
    int sum = htonl(num);//小端转大端

    unsigned  char *p = (char *)&sum;
    printf("%d,%d,%d,%d\n" , *p , *(p+1) , *(p+2) , *(p+3));

    printf("====================\n");

    //ntohs
    b = ntohs(b);//大端转小段
    printf("b : %x\n" , b);//使用 16进制 形式打印b
    //ntohl
    sum = ntohl(sum);//大端转小段
    p = (char *)&sum;
    printf("%d,%d,%d,%d\n" , *p , *(p+1) , *(p+2) , *(p+3));
    return 0;
}