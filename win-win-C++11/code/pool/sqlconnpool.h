/*
 * @Author       : The Lin
 * @Date         : 2022-03-03
 */ 
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    /*获取一个数据库实例*/
    static SqlConnPool *Instance();

    /*获得一个可以使用的mysql对象*/
    MYSQL *GetConn();

    /*释放一个mysql连接对象，即将他重新入队*/
    void FreeConn(MYSQL * conn);

    /*获取空闲的数据库连接对象数*/
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);
              
    /*关闭所有mysql连接*/
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;
    int useCount_;
    int freeCount_;
    /*队列：成员类型为MYSQL **/
    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};


#endif // SQLCONNPOOL_H