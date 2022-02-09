#include "config.h"
#include <getopt.h>
#include <stdlib.h>

/*
    构造函数
*/
Config :: Config(){

    //端口号：default 9006
    PORT = 9006;

    //日志写入方式：default 同步
    LOGWrite = 0;

    //触发组合模式，default listenfd LT + connfd LT
    TRIGMode = 0;

    //listenfd 触发模式，default LT
    LISTENTrigmode = 0;

    //connfd 触发模式，default LT
    CONNTrigmode = 0;

    //优雅关闭链接，default 不使用
    OPT_LINGER = 0;

    //数据库连接池数量，default 8
    sql_num = 8;

    //线程池内的线程数量，default 8
    thread_num = 8;

    //关闭日志，default 不关闭
    close_log = 0;

    //并发模型, default proactor
    actor_model = 0;
}

/*
    parse_arg  解析参数
*/
void Config :: parse_arg(int argc , char *argv[]){
    int opt;
    const char * str = "p:l:m:o:s:t:c:a:";
    while((opt = getopt(argc , argv , str)) != -1){ /*getopt()函数将传递给mian()函数的argc,argv作为参数，
                                                    同时接受字符串参数optstring -- optstring是由选项Option字母组成的字符串*/
        /*为了使用getopt()，我们需要在while循环中不断地调用直到其返回-1为止。
          每一次调用，当getopt()找到一个有效的Option的时候就会返回这个Option字符，并设置optarg*/
        switch(opt){
            case 'p':   //端口
            {
                PORT = atoi(optarg);
                break;
            }
            case 'l':   //日志写入方式
            {
                LOGWrite = atoi(optarg);
                break;
            }
            case 'm':   //触发组合模式
            {
                TRIGMode = atoi(optarg);
                break;
            }
            case 'o':   //优雅关闭链接
            {
                OPT_LINGER = atoi(optarg);
                break;
            }
            case 's':   //数据库连接池的数量
            {
                sql_num = atoi(optarg);
                break;
            }
            case 't':   //线程池内的线程数量
            {
                thread_num = atoi(optarg);
                break;
            }
            case 'c':   //是否关闭日志
            {
                close_log = atoi(optarg);
                break;
            }
            case 'a':   //并发模型选择
            {
                actor_model = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}