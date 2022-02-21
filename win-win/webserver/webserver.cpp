#include "webserver.h"

//构造函数
WebServer::WebServer(){
    //TcpCLient 类对象
    users = new TcpClient[MAX_FD];

    //root 文件夹路径
    char server_path[200];
    //获取当前工作目录路径，到server_path数组中
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);
    //定时器
    users_timer = new client_data[MAX_FD];
}
//析构函数
WebServer::~WebServer(){
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);//关闭管道文件
    close(m_pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}
//初始化
void WebServer::init(int port, string user, string passWord, string databaseName,
                     int log_write, int opt_linger, int trigmode, int sql_num,
                     int thread_num, int close_log, int actor_model)
{
    //将所有子模块，需要用到的信息，保存到成员变量中
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_actormodel = actor_model;
}
//设置 listenfd和connfd的模式组合
void WebServer::trig_mode(){
    // LT + LT 
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}
//日志初始化
void WebServer::log_write(){
    // 0 : 打开日志
    if(0 == m_close_log){
        Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
    }
    // 1： 关闭日志
    else{
        Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
}
//线程池初始化
void WebServer::sql_pool(){
    //初始化数据库连接池
    m_connPool = connection_pool::GetInstance();
    m_connPool->init("localhost", m_user, m_passWord, m_databaseName, 3306, m_sql_num, m_close_log);
    //初始化数据库读取表
    users->initmysql_result(m_connPool);
}
//线程池初始化
void WebServer::thread_pool(){
    //线程池
    m_pool = new threadpool<TcpClient>(m_actormodel, m_connPool, m_thread_num);
}
//监听
void WebServer::eventListen(){
    //网络编程基础步骤
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    //断言函数，如果条件不成立，立即捕捉错误信息，
    //打印错误信息，并终止程序
    assert(m_listenfd >= 0);

    if(0 == m_OPT_LINGER){
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else{
    //优雅关闭连接:
    /* 如果发送缓存中还有数据未发出则其发出去，
     * 并且收到所有数据的ACK之后，发送FIN包，
     * 开始关闭过程。TCP连接线关闭一个方向，
     * 此时另外一个方向还是可以正常进行数据传输
     */
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    //绑定
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET , "192.168.146.128" , (void *)&address.sin_addr);
    //address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);
    //设置端口复用
    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    //监听
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    //定时器，设置超时时间
    utils.init(TIMESLOT);

    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    TcpClient::m_epollfd = m_epollfd;

    //创建一对管道套接字，进行进程间的通信
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    //设置 管道文件 1 为非阻塞
    utils.setnonblocking(m_pipefd[1]);
    //分配给定时器 管道文件 0 用于给管道文件 1通信
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    //设置管道文件 0 的信号和信号处理函数
    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    //闹钟函数，TIMESLOT秒后触发SIGALRM信号
    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

//定时器初始化
void WebServer::timer(int connfd, struct sockaddr_in client_address){
    //客户端类初始化函数
    users[connfd].init(connfd, client_address, m_root, 
                        m_CONNTrigmode, m_close_log, m_user,
                        m_passWord, m_databaseName);
    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，
    //绑定用户数据，将定时器添加到链表中
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    //创建定时器类
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);//获取当前时间
    timer->expire = cur + 3 * TIMESLOT;//设置超时时间
    users_timer[connfd].timer = timer;
    //将定时器添加到链表中
    utils.m_timer_lst.add_timer(timer);
}
//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
void WebServer::adjust_timer(util_timer *timer){
    //获取当前时间
    time_t cur = time(NULL);
    //重新设置超时时间
    timer->expire = cur + 3 * TIMESLOT;
    //定时器链表重新调整
    utils.m_timer_lst.adjust_timer(timer);
    LOG_INFO("%s", "adjust timer once");
}
//定时器超时处理
void WebServer::deal_timer(util_timer *timer, int sockfd){
    //调用定时器超时处理函数
    timer->cb_func(&users_timer[sockfd]);
    //将该定时器移出链表
    if (timer)
    {
        utils.m_timer_lst.del_timer(timer);
    }
    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}
//和客户端建立连接，设置客户端定时器定时
bool WebServer::dealclinetdata(){
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);

    //水平触发：LT模式
    if(0 == m_LISTENTrigmode){
        int connfd = accept(m_listenfd, 
                    (struct sockaddr *)&client_address, 
                    &client_addrlength);
        //客户端连接失败
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        //客户端连接成功：但是服务器已连接数已满
        if(TcpClient::m_user_count >= MAX_FD){
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        //正常客户端连接：设置定时器超时时间和超时处理函数
        timer(connfd, client_address);
    }
    //边缘触发：ET模式
    else{
        while(1){
            int connfd = accept(m_listenfd, 
                        (struct sockaddr *)&client_address, 
                        &client_addrlength);
            //客户端连接失败
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                return false;
            }
            //客户端连接成功：但是服务器已连接数已满
            if (TcpClient::m_user_count >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                return false;
            }
            //正常客户端连接：设置定时器超时时间和超时处理函数
            timer(connfd, client_address);
        }
        return false;
    }
    return true;
}
//处理信号
bool WebServer::dealwithsignal(bool &timeout, bool &stop_server){
    int ret = 0;
    int sig;
    char signals[1024];
    //接受来自定时器的管道文件 m_pipefd[0] 发送过来的信息
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else{
        for(int i = 0; i < ret; ++i){
            switch (signals[i])
            {
                case SIGALRM://超时信号
                {
                    timeout = true;
                    break;
                }
                case SIGTERM://kill信号 || Ctrl + c 
                {
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;
}
//处理读事件
void WebServer::dealwithread(int sockfd){
    //获取客户端定时器
    util_timer *timer = users_timer[sockfd].timer;
    //reactor 模式
    if (1 == m_actormodel){
        if(timer){
            //重新定时器，调整定时器链表
            adjust_timer(timer);
        }
        //若监测到读事件，将该事件放入请求队列
        //参数：state 0 ,即可读去客户端发送的数据
        m_pool->append(users + sockfd, 0);

        while(true){
            if (1 == users[sockfd].improv){
                //超时
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    //proactor
    else{
        if(users[sockfd].read_once()){
            LOG_INFO("deal with the client(%s)", 
                inet_ntoa(users[sockfd].get_address()->sin_addr));
            //若监测到读事件,将该事件放入请求队列
            m_pool->append_p(users + sockfd);

            //重新定时，调整定时器链表
            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}
//处理客户端写事件
void WebServer::dealwithwrite(int sockfd)
{
    //获取客户端定时器
    util_timer *timer = users_timer[sockfd].timer;
    //reactor模式
    if(1 == m_actormodel){
        //重新定时，调整定时器链表
        if (timer)
        {
            adjust_timer(timer);
        }
        //将监测到写事件,将该事件放入请求队列
        m_pool->append(users + sockfd, 1);
        while(true){
            if (1 == users[sockfd].improv)
            {
                //超时
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    //proactor
    else{
        if (users[sockfd].m_write())
        {
            LOG_INFO("send data to the client(%s)", 
                inet_ntoa(users[sockfd].get_address()->sin_addr));
            //重新定时，调整定时器链表
            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else{
            //定时器超时处理函数
            deal_timer(timer, sockfd);
        }
    }
}
//运行函数：主函数调用
void WebServer::eventLoop(){
    bool timeout = false;
    bool stop_server = false;
    while (!stop_server){
        //等待事件触发
        int number = epoll_wait(m_epollfd, events, 
                            MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }
        for (int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;
            //处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                bool flag = dealclinetdata();
                if (false == flag)
                    continue;
            }
            //客户端关闭连接，移除对应的定时器
            else if (events[i].events & 
                    (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                util_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
            //处理信号
            else if ((sockfd == m_pipefd[0]) 
                    && (events[i].events & EPOLLIN)){
                bool flag = dealwithsignal(timeout, stop_server);
                if (false == flag){
                    LOG_ERROR("%s", "dealclientdata failure");
                }
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            //处理客户连接上 服务器需要写入的数据
            else if (events[i].events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }
        //客户端和服务端无信息交互 ：超时
        if (timeout)
        {
            utils.timer_handler();
            LOG_INFO("%s", "timer tick");
            timeout = false;
        }
    }
}
