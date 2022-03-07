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
    /* if (mysql_query(mysql, "SELECT username,passwd FROM user"))
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
    } */
}
//队文件描述设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
// 修改文件描述符，重置socket上的EPOLLONESHOT事件，以确保下一次可读时，EPOLLIN事件能被触发
void modfd(int epollfd, int fd, int ev, int TRIGMode , bool one_shot) {
    epoll_event event;
    event.data.fd = fd;
    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLRDHUP;
    else
        event.events = ev | EPOLLRDHUP;
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
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
    m_method = DEFAULT;
    //m_url = 0;
    //m_version = 0;
    //m_content_length = 0;
    //m_host = 0;
    //m_start_line = 0;
    //m_checked_idx = 0;
    m_read_idx = 0;
    //m_write_idx = 0;
    //cgi = 0;
	m_write_idx = 0;
 	write_file_size = 0;
    m_state = 0;//默认状态为: 读取客户读写入的数据
    timer_flag = 0;//超时flag默认为0
    improv = 0;
	m_fileaddress = NULL;
    //sql_list = new list<string>();
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
bool TcpClient::m_write(){
    return false;
}
//命令：客户端可以继续发送数据
bool TcpClient::ClientContinuSendCMD(int sockfd){
    char * str = "continue";
    int len = strlen(str);
    if( len == write(sockfd,  str, len)){
        return true;
    }
    else{
        return false;
    }
}
//Client请求状态验证成功，Server返回的CMD
bool TcpClient::StateVerifyCMD(int sockfd,const char * cmd){
    int len = strlen(cmd);
    if( len == write(sockfd, cmd, len)){
        return true;
    }
    else{
        return false;
    }
}
//Post状态确定
void TcpClient::PostState(string &str_recvdata)
{
    //判断为POST
    m_method = POST;
    //找到了
    str_recvdata = str_recvdata.substr(14);
    string file_path = "./";
    file_path = file_path + str_recvdata;

    //检查调用进程是否可以对指定的文件执行某种操作
    if (access(file_path.c_str(), F_OK) == 0)
    { //F_OK      测试文件是否存在
        //表示文件存在
        fd = fopen(file_path.c_str(), "w");
        //创建一个用于写入的空文件。如果文件名称与已存在的文件相同，则会删除已有文件的内容，文件被视为一个新的空文件。
    }
    else
    {
        fd = fopen(file_path.c_str(), "a");
        //追加到一个文件。写操作向文件末尾追加数据。如果文件不存在，则创建文件。
    }
    //状态验证成功CMD
    StateVerifyCMD(m_sockfd, "StateVerify");
}
//GET状态确定
void TcpClient::GETState(string &str_recvdata)
{
    //判断为GET
    m_method = GET;
    //截取掉 GET:Filename
    str_recvdata = str_recvdata.substr(13);
    string file_path = "./";
    file_path += str_recvdata; //获得文件的绝对路径
    //判断文件是否存在
    if (access(file_path.c_str(), F_OK) == 0)
    {
        //获取文件属性
        stat(file_path.c_str(), &m_file_stat);
        //保存文件大小
        write_file_size = m_file_stat.st_size;
        if (write_file_size > 0)
        {
            //文件有内容
            //打开文件
            int fd = open(file_path.c_str(), O_RDONLY);
            //将文件进行映射
            m_fileaddress = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            //写入时的当前索引 = 0
            m_write_idx = 0;
            close(fd);
            //发送确认GET命令,附带文件大小
            StateVerifyCMD(m_sockfd, "StateVerify");
        }
    }
}
//获取客户端的请求状态
void TcpClient::GetClientState()
{
    string str_recvdata = m_read_buf;
    //判断是否为 Post
    if(str_recvdata.find("Post:Filename:") != string::npos)
    {
        PostState(str_recvdata);
    }
    //判断是否为 GET
    else if(str_recvdata.find("GET:Filename:") != string::npos)
    {
        GETState(str_recvdata);
    }
    //判断是否为 mysql insert
    else if (str_recvdata.find("MySQL:Insert:") != string::npos)
    {
        //截取出数据
        str_recvdata = str_recvdata.substr(13);
        //调用相关的函数，进行处理
        if(CGIMysqlInertLine(str_recvdata)){
            //若成功，返回结果
            StateVerifyCMD(m_sockfd, "Finish");
        }
        else{
            mysql_commit(mysql);
            StateVerifyCMD(m_sockfd, "fail");
        }
    }
    //判断是否为 mysql query
    else if (str_recvdata.find("MySQL:Query:") != string::npos)
    {
        //截取出 主键: 条码
        str_recvdata = str_recvdata.substr(12);
        //调用相关函数
        if(CGIMysqlQueryLine(str_recvdata)){
            //若成功, 返回条码 + 查询到的数据
            StateVerifyCMD(m_sockfd, str_recvdata.c_str());
        }
        else{
            StateVerifyCMD(m_sockfd, "fail");
        }
    }
    // UPDATE更新数据
    else if (str_recvdata.find("MySQL:UPDATE:") != string::npos)
    {
        //截取
        str_recvdata = str_recvdata.substr(13);
        if(CGIMysqlUPDATERows(str_recvdata)){
            //若成功，返回结果
            StateVerifyCMD(m_sockfd, "Finish");
        }
        else{
            StateVerifyCMD(m_sockfd, "fail");
        }
    }
    // Delete删除数据
    else if (str_recvdata.find("MySQL:Delete:") != string::npos){
        //截取
        str_recvdata = str_recvdata.substr(13);
        if (CGIMysqlDeleteRows(str_recvdata))
        {
            //若成功，返回结果
            StateVerifyCMD(m_sockfd, "Finish");
        }
        else
        {
            StateVerifyCMD(m_sockfd, "fail");
        }
    }
}
//mysql事务管理
bool TcpClient::MySQLTransactionProcess()
{
    //上锁
    m_lock.lock();
    //插入数据库
    //事务提交模式修改：修改数据库提交模式为0
    if (mysql_query(mysql, "SET AUTOCOMMIT = 0"))
    {
        LOG_ERROR("%s", "SET AUTOCOMMIT = 0 fail");
        m_lock.unlock();
        return false;
    }
    //事务开始
    if (mysql_query(mysql, "START TRANSACTION"))
    {
        LOG_ERROR("%s", "START TRANSACTION fail");
        m_lock.unlock();
        return false;
    }
    //设置保留点
    if (mysql_query(mysql, "SAVEPOINT delete1"))
    {
        LOG_ERROR("%s", "SAVEPOINT delete1 fail");
        m_lock.unlock();
        return false;
    }
    int len = sql_list.size();
    int i;
    for (i = 0; i < len; ++i)
    {
        if (mysql_query(mysql, sql_list[i].c_str()))
        {
            //如果失败
            mysql_query(mysql, "ROLLBACK TO delete1");
            LOG_ERROR("%s", "ROLLBACK delete1");
            break;
        }
    }
    //事务提交
    mysql_commit(mysql);
    sql_list.clear();
    //解锁
    m_lock.unlock();
    return true;
    //判断本地 事务是否全部完成
    if(i != len){
        return false;
    }
    else{
        return true;
    }
}
//数据库：删(delete)
bool TcpClient::CGIMysqlDeleteRows(string sql_delete){
    //查找 " 存在的位置
    int index = sql_delete.find(';');
    //pcba表的插入语句
    string sql_step_1 = sql_delete.substr(0, index);

    string sql_step_2 = sql_delete.substr(index + 1, sql_delete.length() - index - 1);

    //return MySQLTransactionProcess(sql_step_1, sql_step_2);
}
//数据库：改(UPDATE)
bool TcpClient::CGIMysqlUPDATERows(string &sql_UPDATE){
    //查找 " 存在的位置
    int index = sql_UPDATE.find(';');
    //pcba表的插入语句
    string sete_1 = "";
    string sete_2 = "";
    if (index != -1)
    {
        sete_1 = sql_UPDATE.substr(0, index);
        sete_2 = sql_UPDATE.substr(index + 1, sql_UPDATE.length() - index - 1);

        //return MySQLTransactionProcess(sete_1, sete_2);
    }
    else{
        //上锁
        m_lock.lock();
        if (!mysql_query(mysql, sql_UPDATE.c_str()))
        {
            //解锁
            m_lock.unlock();
            return true;
        }
        else
        {
            //解锁
            m_lock.unlock();
            return false;
        }
    }
    
    
}
//数据库：查(select)
bool TcpClient::CGIMysqlQueryLine(string &barcode)
{
    string sql_query = "SELECT date, status, pat_1, pat_2, pat_3, pro_1, pro_2, pro_3 \
                        FROM pcba,process_state WHERE pcba.barcode=process_state.barcode \
                        AND pcba.barcode = '" + barcode + "'";
    //执行插叙语句
    barcode = "";
    //上锁
    m_lock.lock();
    //计算起始时间
    int rc;
    struct timespec ts_start, ts_end;
    rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
    if (mysql && !mysql_query(mysql, sql_query.c_str())) //mysql_query 只能检查语法错误，不能返回真实结果
    {
        //获取检索完整的结果集
        MYSQL_RES * result = mysql_store_result(mysql);
        //获取检索结果的一行
        MYSQL_ROW row = mysql_fetch_row(result);
        //计算mysql检索结束后的时间
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);
        printf("mysql_query reports %ld.%09ld seconds\n",
               ts_end.tv_sec - ts_start.tv_sec, ts_end.tv_nsec - ts_start.tv_nsec);
        //row 是一个二级指针
        if(row != NULL){        //可以证明检索成功
            string tmp = "";
            for(int i = 0; i<8; ++i){
                tmp = row[i];
                barcode += (tmp + " ");
            }
            //解锁
            m_lock.unlock();
            //释放掉结果集产生的内存
            mysql_free_result(result);
            return true;
        }
        //释放掉结果集产生的内存
        mysql_free_result(result);
    }
    //解锁
    m_lock.unlock();
    return false;
}
//数据库：增(insert)
bool TcpClient::CGIMysqlInertLine(string ClientData){
    int index ;
    //pcba表的插入语句
    for(int i = 0; i<5; i++){
        index = ClientData.find(';');
        string sql_insert = ClientData.substr(0, index);
        //将解析到的sql入链表
        sql_list.push_back(sql_insert);
        //去除上一个sql
        ClientData = ClientData.substr(index + 1);
    }
    sql_list.push_back(ClientData);

    return MySQLTransactionProcess();
}
//提交事务
bool TcpClient::MySQL_COMMIT(MYSQL *mysql)
{
    //事务提交
    if (mysql_commit(mysql))
    {
        LOG_ERROR("%s", "mysql_commit fail");
        m_lock.unlock();
        return false;
    }
    m_lock.unlock();
    return true;
}
//解除映射
void TcpClient::unmap(){
	if(m_fileaddress){
		//解除映射
		munmap(m_fileaddress, write_file_size);
		m_fileaddress = NULL;
		m_write_idx = 0;
	}
}
//处理 POST 请求
void TcpClient::ClientPOST(){
    string str_recvdata = m_read_buf;
    if (str_recvdata.find("over") != string::npos){
        fflush(fd);
        fclose(fd);
        fd = NULL;
        m_method = DEFAULT;
    }
    else{
	    //cout << str_recvdata << endl;
        if(fputs(str_recvdata.c_str(), fd) < 0){
            printf("写入失败\n");
        }
        else{
        }
        fflush(fd);
        //给客户端发送继续命令
        if(!ClientContinuSendCMD(m_sockfd)){
            printf("发送传输命令失败\n");
        }
    }
}
//处理 GET 请求
void TcpClient::ClientGET(){
	string str_recvdata = m_read_buf;
	if(str_recvdata.find("continue") != string::npos){
		//可以发送的数据长度
		int len = write_file_size - m_write_idx;
		if(len >= 1024){	//可发送文件数据大于1024
			len = write(m_sockfd, m_fileaddress + m_write_idx , 1024);
			m_write_idx += 1024;
		}
		else if(len > 0){//最后一包文件数据
			len = write(m_sockfd, m_fileaddress + m_write_idx , len);
			m_write_idx = write_file_size;
		}
		else{		//无数据，发送结束命令 over
			char * str = "over";
			len = write(m_sockfd, str, strlen(str));
			//解除映射
			unmap();
			//客户端状态设置为 DEFAULT
			m_method = DEFAULT;
		}
	}
}
//处理读到的数据
void TcpClient::process(){
    switch (m_method)
    {
    case DEFAULT:
        GetClientState();//获取客户端的请求状态
        break;
    case POST:
        ClientPOST();//POST模式下的程序
        break;
    case GET:
        ClientGET();//GET模式下的 函数
        break;
    default:
        break;
    }
    m_read_idx = 0;
    memset(m_read_buf, '\0', READ_BUFFER_SIZE); //清空接受缓冲区
    //因为默认使用了 EPOLLONESHOT ，重置socket上的EPOLLONESHOT事件
    modfd(m_epollfd, m_sockfd, EPOLLIN ,m_TRIGMode, true);
}
