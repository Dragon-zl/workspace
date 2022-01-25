#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H

#include <iostream>
#include <mysql/mysql.h>
#include "../lock/locker.h"
#include "../log/log.h"
#include <list>
#include <stdlib.h>

using namespace std;



class connection_pool{
    public:
        MYSQL * GetConnection();            //获取数据库连接
        bool ReleaseConnection(MYSQL * con);//释放当前使用的连接
        int GetFreeConn();                  //获取当前空闲的连接数
        void DestoryPool();                 //销毁所有连接

        //单例模式
        static connection_pool * GetInstance();

        //初始化函数
        void init(string url , string User , string PassWord  ,
                    string DatabaseName , int Port , int MaxConn , int close_log);

    private:
        connection_pool();
        ~connection_pool();

        int m_MaxConn;      //最大连接数
        int m_CurConn;      //当前已使用的连接数
        int m_FreeConn;     //当前空闲的连接数
        
        locker lock;
        list<MYSQL *> connList;     //连接池
        sem  reserve;           //信号量
    public:
        string m_url;           //主机地址
        string m_Port;          //数据库端口号
        string m_User;          //登录数据库用户名
        string m_PassWord;      //登录数据库密码
        string m_DatabaseName;  //使用数据库名
        int m_close_log;        //日志开关
};

class connectionRAII{
    public:
        connectionRAII(MYSQL **con , connection_pool * connPool);
        ~connectionRAII();

    private:
        MYSQL * conRAII;
        connection_pool * poolRAII;
};
#endif