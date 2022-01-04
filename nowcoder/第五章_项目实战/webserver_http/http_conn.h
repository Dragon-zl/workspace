#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>
#include <sys/stat.h>
class http_conn
{
public:
    //HTTP请求方法，但我们只支持GET
    enum METHOD{ GET = 0 , POST , HEAD , PUT , DELETE , TRACE , OPTIONS , CONNECT};
    /*
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE:当前正在分析请求行
        CHECK_STATE_HEADER:     当前正在分析头部字段
        CHECK_STATE_CONTENT:    当前正在解析请求体
    */
    enum CHECK_STATE{ CHECK_STATE_REQUESTLINE = 0 , CHECK_STATE_HEADER , CHECK_STATE_CONTENT};
    /*
        从状态机的三种可能状态，即行的读取状态，分别表示
        1、读取到一个完整的行
        2、行出错
        3、行数据尚且不完整
    */
    enum  LINE_STATUS{ LINE_OK = 0 , LINE_BAD , LINE_OPEN};
    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST              :   请求不完整，需要继续读取客户数据
        GET_REQUEST             :   表示获得了一个完成的客户请求
        BAD_REQUEST             :   表示客户请求语法错误
        NO_RESOURCE             :   表示服务器没有资源
        FORBIDDEN_REQUEST       :   表示客户对资源没有足够的访问权限
        FILE_REQUEST            :   文件请求，获取文件成功
        INTERNAL_ERROR          :   表示服务器内部错误
        CLOSED_CONNECTION       :   表示客户端已经关闭连接了
    */
    enum  HTTP_CODE{ NO_REQUEST, GET_REQUEST , BAD_REQUEST , NO_RESOURCE , FORBIDDEN_REQUEST , FILE_REQUEST ,INTERNAL_ERROR , CLOSED_CONNECTION};

    static int My_epollfd;      //所有的 socket 事件，都共享一个 epoll 实例
    static int My_users_count;    //正在连接的用户数量

    // 文件名的最大长度
    static const int FILENAME_LEN = 200;        
    //读缓冲区，大小
    static const int READ_BUFFER_SIZE  = 2048;
    //写缓冲区，大小
    static const int WRITE_BUFFER_SIZE = 1024;

    http_conn(/* args */);
    ~http_conn();

    //初始化新接收的客户端
    void init(int connect_fd , struct sockaddr_in & client_addr);
    //非阻塞的读
    bool read();
    //非阻塞的写
    bool write();
    //关闭连接
    void close_connect();
    // 对内存映射区执行munmap操作
    void unmap();
    //由线程池中的工作线程调用，处理客户端请求的入口函数
    void process();
    //解析 HTTP 请求
    HTTP_CODE process_read();
    //根据服务器处理HTTP请求的结果，决定返回给客户端的内容
    bool process_write(HTTP_CODE);
    //服务器返回状态行
    bool add_status_line(int  , const char *);
    //服务器返回http头
    bool add_headers(int );
    //头部信息中的：返回内容长度
    bool add_content_length(int );
    //头部信息中的：返回内容文件类型
    bool add_content_type();
    //头部信息中的：是否保持连接
    bool add_linger();
    //头部信息中的：\r\n
    bool add_blank_line();
    bool add_content( const char*  );
    //解析请求首行
    HTTP_CODE parse_request_line(char * text);
    // 往写缓冲中写入待发送的数据
    bool add_response(const char* , ...);
    //解析请求头
    HTTP_CODE parse_headers(char * text);
    //解析请求体
    HTTP_CODE parse_content(char * text);
    HTTP_CODE do_request();
    //解析一行
    LINE_STATUS  parse_line();
    
private:
    int  My_sockfd;             //该 HTTP 连接的socket
    sockaddr_in  My_address;    //通信的socket地址
    
    //读缓冲区
    char My_Read_buf[READ_BUFFER_SIZE];
    //标识读缓冲区已读的客户端数据的 最后一个字节的下标 + 1
    int My_read_index;
    //HTTP请求的消息总长度
    int My_content_length; 
    //当前正在分析的字符在读缓冲区的位置
    int My_check_index;
    //当前正在解析的行的起始位置     
    int My_start_line;
    //主状态机当前所处的状态
    CHECK_STATE  My_check_state;

    char My_write_buf[WRITE_BUFFER_SIZE]; //写缓冲区
    int My_write_idx;                     //写缓冲区中待发送的字节数
    
    struct iovec m_iv[2];                 // 我们将采用writev来执行写操作，所以定义下面两个成员，
                                            //其中m_iv_count表示被写内存块的数量。
    int m_iv_count;
    int bytes_to_send;                  // 将要发送的数据的字节数
    int bytes_have_send;                // 已经发送的字节数


    // 客户请求的目标文件的完整路径，其内容等于 doc_root + m_url, doc_root是网站根目录
    char My_real_file[ FILENAME_LEN ];       
    //请求目标文件的文件名
    char * My_url;
    // 目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息
    struct stat My_file_stat; 
    // 客户请求的目标文件被mmap到内存中的起始位置               
    char* My_file_address;                   
    //协议版本 ， 这里设置只支持 HTTP1.1
    char * My_version;
    //请求方法
    METHOD My_method;
    //主机名
    char * My_host;
    //HTTP请求是否保持连接
    bool  My_linger;
    //初始化 数据
    void init();
    //获取一行的其实位置
    char * get_line(){ return My_Read_buf + My_start_line ;}
    
};







#endif