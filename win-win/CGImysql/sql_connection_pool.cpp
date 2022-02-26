#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

//构造函数
connection_pool :: connection_pool(){
    m_CurConn = 0;//当前已使用的连接数
    m_FreeConn = 0;//当前空闲的连接数
}
//获取一个实例
connection_pool * connection_pool :: GetInstance(){
    static connection_pool connPool;
    return &connPool;
}

//构造初始化
void connection_pool::init(string ip, string User, string PassWord, 
                string DBName, int Port, int MaxConn, int close_log)
{
    m_ip = ip;
    m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	m_close_log = close_log;

    for(int i = 0; i < MaxConn; i++){
        MYSQL * con = NULL;
        //分配初始化与mysql_real_connect()相适应的MySQL对象
        con = mysql_init(con);
        if(NULL == con){
            LOG_ERROR("MySQL mysql_init(Out of memory)");
            exit(1);
        }
        //连接数据库引擎
        con = mysql_real_connect(con, ip.c_str(), User.c_str(),
                PassWord.c_str(), DBName.c_str(), Port, NULL, 0);
        //是否连接失败判断
        if(NULL == con){
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        //往 数据库连接 链表中 尾插法插入一个元素
        connList.push_back(con);
        ++m_FreeConn;
    }
    //信号量变量，初始化 m_FreeConn 个信号
    reserve = sem(m_FreeConn);
    m_MaxConn = m_FreeConn;
}
/*
当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
*/
MYSQL * connection_pool::GetConnection(){
    MYSQL *con = NULL;
    //判断数据库连接 链表中是否有数据
    if(0 == connList.size()){
        return NULL;
    }
    //信号量等待
    reserve.wait();
    //加锁
    lock.lock();
    con = connList.front();
    connList.pop_front();
    //更新使用和空闲连接数
    --m_FreeConn;
    ++m_CurConn;
    lock.unlock();
    return con;
}
/*
是否当前使用的连接: 实际上就是把传入的数据库实例重新入链表，
成为一个空闲数据库连接
*/
bool connection_pool :: ReleaseConnection(MYSQL *con){
    if(NULL == con){
        return false;
    }
    //加锁
    lock.lock();
    connList.push_back(con);//插入链表
    ++m_FreeConn;
    --m_CurConn;
    lock.unlock();
    //给信号量 发送信号
    reserve.post();
    return true;
}
//销毁数据库连接池
void connection_pool::DestroyPool(){
    //加锁
    lock.lock();
    if(connList.size() > 0){
        //定义迭代器变量
        list<MYSQL *>::iterator it;
        for(it = connList.begin(); it != connList.end(); ++it){
            MYSQL * con = *it;
            //关闭数据库连接
            mysql_close(con);
        }
        m_CurConn = 0;
        m_FreeConn = 0;
        //先关闭每个数据库对象连接，再清空链表
        connList.clear();
    }
    //解锁
    lock.unlock();
}
//返回付钱空闲的数据库连接对象数
int connection_pool::GetFreeConn(){
    return this -> m_FreeConn;
}
//析构函数
connection_pool::~connection_pool(){
    DestroyPool();//销毁数据库连接池
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}