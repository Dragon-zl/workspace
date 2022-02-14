#include "http_conn.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>
#include <map>
#include <mysql/mysql.h>

// 定义HTTP响应的一些状态信息
const char* ok_200_title    = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form  = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form  = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form  = "The requested file was not found on this server.\n";

// 网站的根目录
const char* doc_root = "/home/dzl/My_space/workspace/nowcoder/第五章_项目实战/webserver_http/resources";

const char* error_500_form  = "There was an unusual problem serving the requested file.\n";

const char* error_500_title = "Internal Error";

int http_conn::My_epollfd = -1;     
int http_conn::My_users_count = 0;    

//设置文件描述符非阻塞
void  setnonblocking(int fd){
    int old_flag = fcntl( fd , F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl( fd , F_SETFL , new_flag);
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
//添加文件描述符到 epoll 实例中
void  addfd_epoll(int epoll , int fd , bool one_shot){
    epoll_event  event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP ; //EPOLLRDHUP ：监听该事件可以使得当 客户端与服务端异常断开时，
                                            //内核能够检测得到，并自动处理，无需应用层去判断处理
    //防止 同一个 socket 触发两个线程去处理
    if( one_shot ){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epoll , EPOLL_CTL_ADD ,fd , &event);
    //设置文件描述符非阻塞
    setnonblocking(fd);
}
//从epoll实例中删除文件描述符
void  removefd_epoll(int epoll , int fd){
    epoll_ctl( epoll , EPOLL_CTL_DEL , fd , 0);
    close(fd);
}
//修改 epoll 实例中的文件描述符 , 每次修改都要重置 EPOLLONESHOT 事件
void  modfd_epoll(int epoll , int fd , int ev){
    epoll_event  event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl( epoll , EPOLL_CTL_MOD , fd , &event);
}


http_conn::http_conn(/* args */)
{
}

http_conn::~http_conn()
{
}
//初始化新接收的客户端
void http_conn::init(int connect_fd , struct sockaddr_in & client_addr){



    My_sockfd = connect_fd;
    My_address = client_addr;

    //设置端口复用
    int optname = 1;
    setsockopt( My_sockfd , SOL_SOCKET , SO_REUSEADDR , &optname , sizeof(optname));

    //添加到 epoll 实例当中
    addfd_epoll( My_epollfd , My_sockfd , true);
    My_users_count++;   //总的连接用户数 + 1

    //初始化 某些数据
    init();
}
map<string, string> users;
void http_conn :: initmysql_result(connection_pool *connPool){
    //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql , connPool);

    //在user表中检索 username，passwd数据，浏览器端输入
    if(mysql_query(mysql , "SELECT username, passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }
    //从表中检索完整的结果集
    MYSQL_RES * result = mysql_store_result(mysql);
    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);
    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_field(result);
    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while(MYSQL_ROW row = mysql_fetch_row(result)){
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
}

//初始化 数据
void http_conn:: init(){
    bytes_to_send = 0;
    bytes_have_send = 0;
    
    My_check_state = CHECK_STATE_REQUESTLINE;
    My_check_index = 0;
    My_start_line = 0;
    My_read_index = 0;
    My_content_length = 0;
    My_method = GET;
    My_url = 0;
    My_version = NULL;
    My_linger - false;
    My_host = NULL;
    My_write_idx = 0;
    //清空缓冲区
    bzero( My_Read_buf , READ_BUFFER_SIZE);
    bzero(My_write_buf, READ_BUFFER_SIZE);
    bzero(My_real_file, FILENAME_LEN);
}
//初始化连接,外部调用初始化套接字地址
void http_conn::init(int sockfd, const sockaddr_in &addr, char *root, int TRIGMode,
                     int close_log, string user, string passwd, string sqlname){
    
    My_sockfd = sockfd;
    My_address = addr;

    addfd(My_epollfd, sockfd, true, m_TRIGMode);
    My_users_count++;
    //当浏览器出现连接重置时，
    //可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root = root;
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name,sqlname.c_str());

    init();
}
//循环读取客户端数据，直到无数据可读或对方关闭连接
//非阻塞 ET工作模式，需要一次性将数据读完
bool http_conn:: read_once(){
    if(My_read_index >= READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read = 0;
    //LT读取数据
    if(0 == m_TRIGMode){
        bytes_read = recv(My_sockfd, My_Read_buf + My_read_index, 
                            READ_BUFFER_SIZE - My_read_index, 0);
        My_read_index += bytes_read;
        if(bytes_read <= 0){
            return false;
        }
        return true;
    }
    //ET读数据
    else{
        while(true){
            bytes_read = recv(My_sockfd, My_Read_buf + My_read_index, 
                            READ_BUFFER_SIZE - My_read_index, 0);
            if(bytes_read == -1){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    break;
                }
                return false;
            }
            else if(bytes_read == 0){
                return false;
            }
            My_read_index += bytes_read;
        }
        return true;
    }
}

//非阻塞的读
bool http_conn:: read(){
    //判断读缓冲区是否 已满
    if( My_read_index >= READ_BUFFER_SIZE ){
        return false;
    }
    //读到的字节
    int byte_read = 0;
    while(true){
        // 从m_read_buf + m_read_idx索引出开始保存数据，大小是READ_BUFFER_SIZE - m_read_idx
        byte_read = recv( My_sockfd , My_Read_buf + My_read_index , 
            READ_BUFFER_SIZE - My_read_index, 0);
        if( -1 == byte_read ){
            if( errno == EAGAIN || errno == EWOULDBLOCK){
                //没有数据
                break;
            }
            return false;
        }
        else if( 0 == byte_read){
            //对方关闭连接
            return false;
        }
        My_read_index += byte_read;
    }
    printf("读取到了数据 %s\n" , My_Read_buf );
    return true;
}
//解析 HTTP 请求
http_conn:: HTTP_CODE http_conn:: process_read(){
    //初始化状态
    LINE_STATUS  line_status = LINE_OK;
    HTTP_CODE  ret = NO_REQUEST;
    char * text = 0;
    while( ((My_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK))
            || ((line_status = parse_line()) == LINE_OK)){
                /*进入循环的条件：
                        解析到了请求体 并且 行状态为 OK
                        或者
                        解析到了一行完整的数据
                */
                //获取一行数据
                text = get_line();
                My_start_line = My_check_index;
                printf("got  1  http  line : %s\n" , text);
                
                switch (My_check_state)
                {
                    case CHECK_STATE_REQUESTLINE:   //分析请求行
                    {
                        //解析，返回结果
                        ret = parse_request_line(text);
                        if( ret == BAD_REQUEST){
                            return BAD_REQUEST; //如果语法错误，直接退出
                        }
                        break;
                    }
                    case CHECK_STATE_HEADER:        //分析头部字段
                    {
                        ret = parse_headers(text);
                        if( ret == BAD_REQUEST){
                            return BAD_REQUEST; //如果语法错误，直接退出
                        }
                        else if(ret == GET_REQUEST){//完整的请求头
                            return do_request();
                        }
                    }
                    case CHECK_STATE_CONTENT:       //分析请求体
                    {
                        ret = parse_content(text);
                        if( ret == BAD_REQUEST){
                            return BAD_REQUEST; //如果语法错误，直接退出
                        }
                        else if(ret == GET_REQUEST){//完整的请求头
                            return do_request();
                        }
                        line_status = LINE_OPEN;
                        break;
                    }
                    default:
                    {
                        //return INTERNAL_ERROR;//内部错误
                        return INTERNAL_ERROR;
                    }
                        
                }
                
            }
            return NO_REQUEST;
}
//解析HTTP请求行，获取请求方法，目标URL，HTTP版本
http_conn:: HTTP_CODE http_conn:: parse_request_line(char * text){

    //这里最高效的方法是使用正则表达式去解析，这里牛客网使用的是传统的方法
    // GET /index.html HTTP/1.1
    My_url = strpbrk(text, " \t"); // 判断第二个参数中的字符哪个在text中最先出现
    if (! My_url) { 
        return BAD_REQUEST;
    }
    // GET\0/index.html HTTP/1.1
    *My_url++ = '\0';    // 置位空字符，字符串结束符
    char* method = text;
    if ( strcasecmp(method, "GET") == 0 ) { // 忽略大小写比较
        My_method = GET;
    } else {
        return BAD_REQUEST;
    }
    // /index.html HTTP/1.1
    // 检索字符串 str1 中第一个不在字符串 str2 中出现的字符下标。
    My_version = strpbrk( My_url, " \t" );
    if (!My_version) {
        return BAD_REQUEST;
    }
    *My_version++ = '\0';
    if (strcasecmp( My_version, "HTTP/1.1") != 0 ) {
        return BAD_REQUEST;
    }
    /**
     * http://192.168.110.129:10000/index.html
    */
    if (strncasecmp(My_url, "http://", 7) == 0 ) {   
        My_url += 7;
        // 在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。
        My_url = strchr( My_url, '/' );
    }
    if ( !My_url || My_url[0] != '/' ) {
        return BAD_REQUEST;
    }
    My_check_state = CHECK_STATE_HEADER; // 检查状态变成检查头
    return NO_REQUEST;
}
//解析HTTP请求行的头部信息，
http_conn:: HTTP_CODE http_conn:: parse_headers(char * text){
    //遇到空行，表示头部字段解析完毕
    if( text[0] == '\0'){
        //如果 HTTP 请求有消息体，则还需要读取 m_content 字节的消息体
        //状态机转移到 CHECK_STATE_CONTENT 状态
        if (My_content_length != 0 ) {
            My_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        // 否则说明我们已经得到了一个完整的HTTP请求
        return GET_REQUEST;
    }else if ( strncasecmp( text, "Connection:", 11 ) == 0 ) {
        // 处理Connection 头部字段  Connection: keep-alive
        text += 11;
        text += strspn( text, " \t" );
        if ( strcasecmp( text, "keep-alive" ) == 0 ) {
            My_linger = true;
        }
    }else if ( strncasecmp( text, "Content-Length:", 15 ) == 0 ) {
        // 处理Content-Length头部字段
        text += 15;
        text += strspn( text, " \t" );
        My_content_length = atol(text);
    }else if ( strncasecmp( text, "Host:", 5 ) == 0 ) {
        // 处理Host头部字段
        text += 5;
        text += strspn( text, " \t" );
        My_host = text;
    } else {
        printf( "oop! unknow header %s\n", text );
    }
}
// 我们没有真正解析HTTP请求的消息体，只是判断它是否被完整的读入了
http_conn:: HTTP_CODE http_conn:: parse_content(char * text){
    if ( My_read_index >= ( My_content_length + My_check_index ) )
    {
        text[ My_content_length ] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}
// 当得到一个完整、正确的HTTP请求时，我们就分析目标文件的属性，
// 如果目标文件存在、对所有用户可读，且不是目录，则使用mmap将其
// 映射到内存地址m_file_address处，并告诉调用者获取文件成功
http_conn:: HTTP_CODE http_conn:: do_request(){
    //  /home/dzl/workspace/workspace/nowcoder/第五章_项目实战/nowcoder/webserver/resources
    strcpy( My_real_file, doc_root );
    int len = strlen( doc_root );
    strncpy( My_real_file + len, My_url, FILENAME_LEN - len - 1 );
    // 获取m_real_file文件的相关的状态信息，-1失败，0成功
    if ( stat( My_real_file, &My_file_stat ) < 0 ) {
        printf("%s\n" , My_real_file);
        return NO_RESOURCE;
    }
    // 判断访问权限
    if ( ! ( My_file_stat.st_mode & S_IROTH ) ) {
        return FORBIDDEN_REQUEST;
    }
    // 判断是否是目录
    if ( S_ISDIR( My_file_stat.st_mode ) ) {
        return BAD_REQUEST;
    }
    // 以只读方式打开文件
    int fd = open( My_real_file, O_RDONLY );
    // 创建内存映射
    My_file_address = ( char* )mmap( 0, My_file_stat.st_size, 
                                    PROT_READ, MAP_PRIVATE, fd, 0 );
    close( fd );
    return FILE_REQUEST;
}
//解析一行，判断依据 \r\n
http_conn::  LINE_STATUS  http_conn:: parse_line(){
    char  temp;
    for( ; My_check_index < My_read_index ; ++My_check_index ){
        temp = My_Read_buf[My_check_index];
        if( temp == '\r'){
            if( My_check_index + 1 == My_read_index){
                return LINE_OPEN;       //获取的数据不全
            }
            else if( My_Read_buf[My_check_index + 1] == '\n'){
                //为了那个获取到这一行，即有字符串结束符，将\r\n全部置为 \0
                My_Read_buf[My_check_index++] = '\0';
                My_Read_buf[My_check_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(temp == '\n'){
            if( My_check_index > 1 && My_Read_buf[My_check_index - 1] == '\r'){
                My_Read_buf[My_check_index-1] = '\0';
                My_Read_buf[My_check_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
//非阻塞的写
bool http_conn:: write(){
    int temp = 0;
    if ( bytes_to_send == 0 ) {
        // 将要发送的字节为0，这一次响应结束。
        modfd_epoll( My_epollfd, My_sockfd, EPOLLIN ); 
        init();
        return true;
    }
    while(1) {
        // 分散写
        temp = writev(My_sockfd, m_iv, m_iv_count);
        if ( temp <= -1 ) {
            // 如果TCP写缓冲没有空间，则等待下一轮EPOLLOUT事件，虽然在此期间，
            // 服务器无法立即接收到同一客户的下一个请求，但可以保证连接的完整性。
            if( errno == EAGAIN ) {
                modfd_epoll( My_epollfd, My_sockfd, EPOLLOUT );
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;

        if (bytes_have_send >= m_iv[0].iov_len)
        {
            //头已发送完毕
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = My_file_address + (bytes_have_send - My_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else
        {
            m_iv[0].iov_base = My_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - temp;
        }

        if (bytes_to_send <= 0)
        {
            // 没有数据要发送了
            unmap();
            modfd_epoll( My_epollfd, My_sockfd, EPOLLIN ); 

            if (My_linger)
            {
                init();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
// 对内存映射区执行munmap操作
void http_conn::unmap() {
    if( My_file_address )
    {
        munmap( My_file_address, My_file_stat.st_size );
        My_file_address = 0;
    }
}

//由线程池中的工作线程调用，处理客户端请求的入口函数
void http_conn:: process(){
    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    printf("parse  request, create  response\n");
    //生成响应
    bool write_ret = process_write( read_ret );
    if ( !write_ret ) {
        close_connect();
    }
    modfd_epoll( My_epollfd, My_sockfd, EPOLLOUT);
}
//根据服务器处理HTTP请求的结果，决定返回给客户端的内容
bool http_conn :: process_write(HTTP_CODE ret){
    switch (ret)
    {
        case INTERNAL_ERROR:    //服务器内部错误
            //服务器返回 HTTP 请求结果的状态
            add_status_line( 500, error_500_title );
            //服务器返回 HTTP 协议请求行头部信息
            add_headers( strlen( error_500_form ) );   
            if ( !add_content( error_500_form ) ) {
                    return false;
            } 
            printf("INTERNAL_ERROR\n");
            break;
        case BAD_REQUEST:   //客户端请求语法错误
            add_status_line( 400, error_400_title );
            add_headers( strlen( error_400_form ) );
            if ( ! add_content( error_400_form ) ) {
                return false;
            }
            printf("BAD_REQUEST\n");
            break;
        case FORBIDDEN_REQUEST: //客户端对资源没有足够的访问权限
            add_status_line( 403, error_403_title );
            add_headers(strlen( error_403_form));
            if ( ! add_content( error_403_form ) ) {
                return false;
            }
            printf("FORBIDDEN_REQUEST\n");
            break;
        case NO_RESOURCE:    //服务器没有资源
            add_status_line( 404, error_404_title );
            add_headers( strlen( error_404_form ) );
            if ( ! add_content( error_404_form ) ) {
                return false;
            }
            printf("NO_RESOURCE\n");
            break;
        case FILE_REQUEST:  //文件请求，获取文件成功
            add_status_line(200, ok_200_title );
            add_headers(My_file_stat.st_size);
            m_iv[ 0 ].iov_base = My_write_buf;
            m_iv[ 0 ].iov_len = My_write_idx;
            m_iv[ 1 ].iov_base = My_file_address;
            m_iv[ 1 ].iov_len = My_file_stat.st_size;
            m_iv_count = 2;
            bytes_to_send = My_write_idx + My_file_stat.st_size;
            printf("FILE_REQUEST\n");
            return true;
        default:
            return false;
            break;
    }
}
bool http_conn::add_content( const char* content )
{
    return add_response( "%s", content );
}
bool http_conn :: add_headers(int content_len){
    add_content_length(content_len);
    add_content_type();
    add_linger();
    return add_blank_line();
}
bool http_conn::add_blank_line()
{
    return add_response( "%s", "\r\n" );
}

bool http_conn::add_linger()
{
    return add_response( "Connection: %s\r\n", ( My_linger == true ) ? "keep-alive" : "close" );
}
bool http_conn::add_content_length(int content_len) {
    return add_response( "Content-Length: %d\r\n", content_len );
}
bool http_conn::add_content_type() {
    return add_response("Content-Type:%s\r\n", "text/html");
}

//服务器返回状态行
bool http_conn :: add_status_line(int  status, const char * title){
    return add_response( "%s %d %s\r\n", "HTTP/1.1", status, title );
}
// 往写缓冲中写入待发送的数据
bool http_conn :: add_response(const char* format, ...){
    if( My_write_idx >= WRITE_BUFFER_SIZE)  //写缓冲区待发送的字节数 >= 写缓冲区大小
    {
        return false;
    }
    va_list  arg_list;
    va_start( arg_list , format);
    int len = vsnprintf(My_write_buf + My_write_idx ,  WRITE_BUFFER_SIZE - 1 - My_write_idx , format , arg_list);
    if( len >= (WRITE_BUFFER_SIZE - 1 - My_write_idx)){ //如果需要发送的字节数，大于缓冲区空间剩余字节数
        return false;
    }
    My_write_idx += len;
    va_end(arg_list);
    return true;
}
//关闭连接
void http_conn:: close_connect(){
    if( -1 != My_sockfd){
        //将套接字从 epoll实例中哦移除
        removefd_epoll( My_epollfd , My_sockfd );
        My_sockfd = -1;
        //连接用户数 减 1
        My_users_count--;
    }
}
