/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */

#include "webserver.h"

using namespace std;

WebServer::WebServer(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
            timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
    {
        /*获取当前目录的绝对路径*/
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);

    Client::userCount = 0;
    Client::srcDir = srcDir_;
    /*获取一个SqlConnPool连接实例： 登录本地数据库引擎，创建 connPoolNum 个MySQL对象入队等待调用*/
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    /*注册监听、等待连接文件描述符为 ET 或者 LT模式*/
    InitEventMode_(trigMode);
    if(!InitSocket_()) { isClose_ = true;}

    if(openLog) {
        /*日志初始化*/
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", Client::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    Client::isET = (connEvent_ & EPOLLET);
}
/*等待客户单连接*/
void WebServer::Start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    while(!isClose_) {
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        /*epoll_wait 返回就绪文件描述符个数*/
        int eventCnt = epoller_->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            /* 获取文件描述符 */
            int fd = epoller_->GetEventFd(i);
            /* 获取就绪文件描述符的就绪事件*/
            uint32_t events = epoller_->GetEvents(i);
            /*有新的客户端连接*/
            if(fd == listenFd_) {
                /*处理客户端连接*/
                DealListen_();
            }
            /*客户端关闭连接*/
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                /*关闭客户端函数*/
                CloseConn_(&users_[fd]);
            }
            /*有读事件 响应*/
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            /*写事件 响应*/
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}
/*服务器向客户端发送连接失败信息：原因服务器连接客户端数已满*/
void WebServer::SendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}
/*处理客户端主动断开连接*/
void WebServer::CloseConn_(Client* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    /*从epoll集合中删除*/
    epoller_->DelFd(client->GetFd());
    /*关闭连接*/
    client->Close();
}
/*添加新的客户端连接*/
void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    
    if(timeoutMS_ > 0) {
        /*
            bind:动态生成新的函数
        */
        /*望 定时器堆 中添加新成员*/
        timer_->add(fd, timeoutMS_, bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    /*添加输入事件、ET或LT模式*/
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    /*非阻塞*/
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}
/*处理监听文件描述符：等待连接*/
void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(Client::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        /*增加一个客户端*/
        AddClient_(fd, addr);
    } while(listenEvent_ & EPOLLET);
}
/*处理读事件：接受客户端连接文件描述符发送过来的数据*/
void WebServer::DealRead_(Client* client) {
    //判断传入参数是否为空
    assert(client);
    /*重新调整 定时器容器*/
    ExtentTime_(client);
    /*线程池中添加任务*/
    threadpool_->AddTask(bind(&WebServer::OnRead_, this, client));
}
/*处理写事件：向传入的客户端连接文件描述符发送数据*/
void WebServer::DealWrite_(Client* client) {
    assert(client);
    /*重新调整客户端对应的定时器*/
    ExtentTime_(client);
    threadpool_->AddTask(bind(&WebServer::OnWrite_, this, client));
}
/*重新调整客户端对应的定时器*/
void WebServer::ExtentTime_(Client* client) {
    assert(client);
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_); }
}
/*读取客户端数据*/
void WebServer::OnRead_(Client* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    /*调用 http类read函数*/
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    /*读取数据后的数据分析处理函数*/
    OnProcess(client);
}
/*读取数据后的数据分析处理函数*/
void WebServer::OnProcess(Client* client) {
    if(client->process()) {
        /*注册写事件*/
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else {
        /*注册读事件*/
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}
/*写数据到客户端*/
void WebServer::OnWrite_(Client* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    /*调用客户端 write 函数*/
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        /* if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        } */
    }
    else if(ret < 0) {
        /*系统提示再写一次的事件： EAGAIN*/
        if(writeErrno == EAGAIN) {
            /* 发送失败 重新注册  EPOLLOUT 事件*/
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    /*如果 即未发送成功， 又不是 因为 EAGAIN 事件导致，主动关闭客户端连接*/
    CloseConn_(client);
}

/* Create listenFd */
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port:%d error!",  port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger = { 0 };
    if(openLinger_) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }
    /*设置配好的 openLinger_： 优雅关闭连接*/
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。- 防止服务器重启时之前绑定的端口还未释放
                                        - 程序突然退出而系统没有释放端口
                                        SO_REUSEADDR允许您的服务器   绑定到a中的地址   TIME_WAIT状态。 
                                        此套接字选项告诉内核即使此端口忙（处于TIME_WAIT状态），
                                        也要继续并重新使用它。如果它很忙，但是有另一个状态，
                                        你仍然会得到一个已经处于使用中的地址错误。如果您的服务器已关闭，
                                        然后在其端口上的套接字仍处于活动状态时立即重新启动，则此功能非常有用。 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    /*设置为非阻塞*/
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}
/*设置文件描述符为非阻塞：ET模式下必须是非阻塞*/
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}


