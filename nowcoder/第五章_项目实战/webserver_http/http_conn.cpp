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
// 网站的根目录
const char* doc_root = "/home/dzl/My_space/workspace/nowcoder/第五章_项目实战/nowcoder/webserver/resources";
int http_conn::My_epollfd = -1;     
int http_conn::My_users_count = 0;    

//设置文件描述符非阻塞
void  setnonblocking(int fd){
    int old_flag = fcntl( fd , F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl( fd , F_SETFL , new_flag);
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
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
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
//初始化 数据
void http_conn:: init(){
    My_check_state = CHECK_STATE_REQUESTLINE;
    My_check_index = 0;
    My_start_line = 0;
    My_read_index = 0;
    My_content_length = 0;
    My_method = GET;
    My_url = NULL;
    My_version = NULL;
    My_linger - false;
    My_host = NULL;
    //清空缓冲区
    bzero( My_Read_buf , READ_BUFFER_SIZE);
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
                return NO_REQUEST;
            }
}
//解析HTTP请求行，获取请求方法，目标URL，HTTP版本
http_conn:: HTTP_CODE http_conn:: parse_request_line(char * text){

    //这里最高效的方法是使用正则表达式去解析，这里牛客网使用的是传统的方法

    //GET /index.html HTTP/1.1
    My_url = strpbrk( text , " \t");
    if( !My_url ){
        return BAD_REQUEST;
    }
    //GET\0/index.html HTTP/1.1
    *My_url++ = '\0';

    char * method = text;
    //不区分大小写的字符串比较函数
    if( strcasecmp(method , "GET") == 0){
        My_method = GET;
    }
    else{
        return BAD_REQUEST;
    }
    My_version = strpbrk( text , " \t");
    if( !My_version ){
        return BAD_REQUEST;
    }
    //GET /index.html\0HTTP/1.1
    *My_version++ = '\0';
    if( strcasecmp(My_version , "HTTP/1.1") != 0){
        return BAD_REQUEST;
    }
    //http://192.168.1.1:10000/index.html
    //比较My_url前 7 个字符是否 为 "http://"
    if( strncasecmp( My_url , "http://" , 7) == 0){
        My_url += 7;
        My_url = strchr( My_url , '/'); //index.html
    }

    if( !My_url || My_url[0] != '/'){
        return BAD_REQUEST;
    }
    //主状态机的状态改为改为检查请求头
    My_check_state = CHECK_STATE_HEADER;
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
    printf("一次性写入数据\n");
    return true;
}
//由线程池中的工作线程调用，处理客户端请求的入口函数
void http_conn:: process(){
    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    printf("parse  request, create  response\n");
    //生成响应
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
