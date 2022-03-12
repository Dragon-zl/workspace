/*
 * @Author       : mark
 * @Date         : 2021-03-10
 */ 
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../client/client.h"

using namespace std;

class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    /*等待客户单连接*/
    void Start();

private:
    /* Create listenFd */
    bool InitSocket_(); 
    /*注册监听、等待连接文件描述符为 ET 或者 LT模式*/
    void InitEventMode_(int trigMode);
    /*添加新的客户端连接*/
    void AddClient_(int fd, sockaddr_in addr);
    /*处理监听文件描述符：等待连接*/
    void DealListen_();
    /*处理写事件：向传入的客户端连接文件描述符发送数据*/
    void DealWrite_(Client* client);
    /*处理读事件：接受客户端连接文件描述符发送过来的数据*/
    void DealRead_(Client* client);

    /*服务器向客户端发送连接失败信息：原因服务器连接客户端数已满*/
    void SendError_(int fd, const char*info);
    /*重新调整客户端对应的定时器*/
    void ExtentTime_(Client* client);
    /*处理客户端主动断开连接*/
    void CloseConn_(Client* client);

    /*读取客户端数据*/
    void OnRead_(Client* client);
    /*写数据到客户端*/
    void OnWrite_(Client* client);
    /*读取数据后的数据分析处理函数*/
    void OnProcess(Client* client);

    static const int MAX_FD = 65536;

    /*设置文件描述符为非阻塞：ET模式下必须是非阻塞*/
    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;
   
   /*定时器类*/
    unique_ptr<HeapTimer> timer_;
    /*数据库类*/
    unique_ptr<ThreadPool> threadpool_;
    /*epoll指针*/
    unique_ptr<Epoller> epoller_;
    /*客户端map*/
    unordered_map<int, Client> users_;
};


#endif //WEBSERVER_H