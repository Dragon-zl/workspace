#include "tcpclient.h"
#include <mysql/mysql.h>
#include <fstream>
#include <regex>

locker m_lock;
map<string, string> users;

int TcpClient::m_user_count = 0;
int TcpClient::m_epollfd = -1;

void TcpClient::initmysql_result(connection_pool *connPool)
{
    //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        //LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);
    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);
    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result)){
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
}
//队文件描述设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
//初始化连接，外部调用初始化套接字地址
void TcpClient::init(int sockfd, const sockaddr_in &addr,
                        char *root, int TRIGMode, int close_log,
                        string user, string passwd, string sqlname)
{
    m_sockfd = sockfd;
    m_address = addr;

    //注册事件
    addfd(m_epollfd, sockfd, true, m_TRIGMode);
    m_user_count++;//客户端数目++

    fd = NULL;
    doc_root = root;
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}
//初始化新接受的连接
//check_state默认为分析请求行状态
void TcpClient::init(){

    mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    //m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = POST;//默认为上传文件
    //m_url = 0;
    //m_version = 0;
    //m_content_length = 0;
    //m_host = 0;
    //m_start_line = 0;
    //m_checked_idx = 0;
    m_read_idx = 0;
    //m_write_idx = 0;
    //cgi = 0;

    m_state = 0;//默认状态为: 读取客户读写入的数据
    timer_flag = 0;//超时flag默认为0
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool TcpClient::read_once(){
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;
    //LT读取数据
    if (0 == m_TRIGMode){
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, 
                            READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    //ET读数据
    else{
        while (true){
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, 
                            READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}
//写 赞为实现
bool TcpClient::write(){
    return false;
}
//处理读取到的数据
void TcpClient::process(){
    string str_recvdata = m_read_buf;
    if(fd == NULL){
        //上传文件的协议，接受到文件名
        if (str_recvdata.find("Filename:") != string::npos)
        {
            //找到了
            str_recvdata = str_recvdata.substr(9);
            string file_path = "./";
            file_path = file_path + str_recvdata;
            fd = fopen(file_path.c_str() , "a");//追加文件，没有则创建
        }
    }
    else if (str_recvdata.find("over") != string::npos){
        fflush(fd);
        fclose(fd);
        fd = NULL;
    }
    else{
        printf("传输一次\n");
        if(0 == fwrite(m_read_buf, sizeof(m_read_buf), 1, fd)) //数据写入文件
        {
            printf("写入失败\n");
        }
        fflush(fd);
    }
    m_read_idx = 0;
    memset(m_read_buf, '\0', READ_BUFFER_SIZE); //清空接受缓冲区
}
