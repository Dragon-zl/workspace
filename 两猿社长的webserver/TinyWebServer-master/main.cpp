#include "config.h"

int main(int argc, char *argv[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "root";
    string passwd = "19991011qwe#QWE";
    string databasename = "WebserverDB";

    //命令行解析
    Config config;
    config.parse_arg(argc, argv);


    WebServer server;

    //初始化
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num, 
                config.close_log, config.actor_model);
    

    //日志
    server.log_write();

    printf("初始化LOG成功\n");
    //数据库
    server.sql_pool();

    printf("初始化SQL成功\n");
    //线程池
    server.thread_pool();

    //触发模式
    server.trig_mode();

    //监听
    server.eventListen();

    //运行
    server.eventLoop();

    printf("运行结束\n");
    return 0;
}