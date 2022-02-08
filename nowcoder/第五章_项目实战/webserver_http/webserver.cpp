#include "webserver.h"

WebServer :: WebServer(){
    //http_conn 类对象
    users = new http_conn[MAX_FD];
    //root 文件夹路径
    char server_path[200];
    //将当前工作目录的绝对路径名 复制到指向的数组中
    getcwd(server_path , 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy( m_root , server_path);
    strcat(m_root , root);

    //定时器
    users_timer = new client_data[MAX_FD];
}

WebServer :: ~WebServer(){
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}

void WebServer :: init( int port, string user , string passWord ,
                        string databaseName , int log_write , 
                        int opt_linger, int trigmode, int sql_num,
                        int thread_num, int close_log, int actor_model)
{
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

void WebServer :: trig_mode(){
    //LT + LT
    if(0 == m_TRIGMode){
        m_LISTENTrigmode = 0;
        m_CONNTrigmode   = 0;
    }
    //LT + ET
    else if(1 == m_TRIGMode){
        m_LISTENTrigmode = 0;
        m_CONNTrigmode   = 1;
    }
    //ET + LT
    else if(2 == m_TRIGMode){
        m_LISTENTrigmode = 1;
        m_CONNTrigmode   = 0;
    }
    //ET + ET
    else if(3 == m_TRIGMode){
        m_LISTENTrigmode = 1;
        m_CONNTrigmode   = 1;
    }
}

void WebServer :: log_write(){
    if(0 == m_close_log){
        //初始化日志
        if(1 == m_log_write){
            Log :: get_instance() -> init("./ServerLog", m_close_log, 2000, 800000, 800);
        }
        else{
            Log :: get_instance() -> init("./ServerLog", m_close_log, 2000, 800000, 0);
        }
    }
}
void WebServer :: sql_pool(){
    //初始化数据库连接池
    m_connPool = connection_pool::GetInstance();
    m_connPool -> init("localhost", m_user, m_passWord, m_databaseName, 
                        3306, m_sql_num, m_close_log);
    //初始化数据库读取表
    users -> initmysql_result(m_connPool);
}
void WebServer :: thread_pool(){
    //线程池
    m_pool = new threadpool<http_conn>(m_actormodel, m_connPool,m_thread_num);
}
void WebServer :: eventListen(){
    //网络编程基础步骤
    m_listenfd = socket(PF_INET , SOCK_STREAM , 0);
    assert(m_listenfd >= 0);//检查是否出现错误

    //设置端口复用
    if(0 == m_OPT_LINGER){//优雅关闭连接
        struct linger tmp = {0 , 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if(1 == m_OPT_LINGER){
        struct linger tmp = {1 , 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address , sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret > 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    utils.init(TIMESLOT);    

    //epoll 创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::My_epollfd = m_epollfd;

    //创建两个新的套接字
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    utils.setnonblocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}