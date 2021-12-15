#include <stdio.h>
#include <arpa/inet.h>
int main(){


    char buf[] = "192.168.1.6";
    unsigned int num = 0;
    //将点分十进制的字符串ip转换为网络子节序整数
    inet_pton( AF_INET , buf , &num);//转换
    unsigned  char * p = (char *)&num;
    printf("%d,%d,%d,%d\n" , *p , *(p+1) , *(p+2) , *(p+3) );

    printf("========================\n");
    //再转换回来
    char ip[16] = "";
    inet_ntop( AF_INET , (void *)&num , ip , sizeof(ip));
    printf("%s\n" , ip);
    return 0;
}