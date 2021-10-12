
/*
    这个程序showenv.c使用environ变量打印环境变量
*/
#include <stdio.h>
#include <stdlib.h>
extern  char ** environ;
int main()
{
    char ** env = environ;
    while( *env )
    {
        fprintf( stdout , "%s\n" , *env);
        env++;
    }
    exit(0);
}
/*
这个程序遍历environ变量（一个以null结尾的字符串数组），并打印出整个环境。

当在Linux系统中运行该程序时，你将得到如下的输出（略做删减）。
这些变量的数目、出现顺序和值取决于操作系统的版本、所用的shell以及程序运行时的用户设置
*/