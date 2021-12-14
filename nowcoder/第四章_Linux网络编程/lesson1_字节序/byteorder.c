/*
    案例：
        测试本台计算机 是大端还是小端口
    小端：
        低地址存低字节 ， 高地址存高字节
*/
#include <stdio.h>
//定义联合体
union 
{
    short  value;
    char  bytes[sizeof(short)];
}test;

int main(){

    test.value = 0x0102;
    if( test.bytes[0] == 2){
        printf("小端字节序\n");
    }
    else{
        printf("大端字节序\n");
    }
    //---------------------------------------
    int a = 1;//0x00000001
    printf("char b = %d\n" , *(char *)&a);
    return 0;
}