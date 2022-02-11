#include "./http/http_conn.h"
#include "./threadpool/threadpool.h"
#include "config.h"
#include "./log/log.h"

#include <cstdio>
#include <libgen.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>


#define  MAX_FD  65535              //最多运行 最大客户端连接并发量
#define  MAX_EVENT_NUMBER  1000     //监听的事件数的最大值
//注册信号处理函数
void  addsig(int sig , void(handle)(int) ){
    struct sigaction sa;
    //清空
    memset( &sa , '\0' , sizeof(sa));
    //参数即为信号处理函数
    sa.sa_handler = handle;
    //设置阻塞信号集
    sigfillset(&sa.sa_mask);
    //默认使用 sa_handler 处理函数
    sa.sa_flags = 0;
    sigaction( sig , &sa , NULL);
}
//添加文件描述符到 epoll 实例中
extern  void  addfd_epoll(int epoll , int fd , bool one_shot);
//从epoll实例中删除文件描述符
extern  void  removefd_epoll(int epoll , int fd);
//修改 epoll 实例中的文件描述符
extern  void  modfd_epoll(int epoll , int fd , int ev);
int main(int argc , char * argv[]){

    //需要修改的数据库信息，登陆名，密码，库名
    string user = "root";
    string passwod = "root";
    string databasename = "qgydb";

    //命令行解析
    Config config;
    //调用解析函数
    config.parse_arg(argc , argv);








    //初始化日志
    Log::get_instance()->init("./ServerLog", 0, 2000, 800000, 0);
    
    //获取端口号
    int port = config.PORT;

    //对sigpie信号进行注册处理  
    //注册为忽略该信号: 原因：读端全部关闭 ，对管道进行write的进程会收到一个信号 SIGPIPE，通常会导致进程异常终止。
    addsig( SIGPIPE , SIG_IGN);

    /* //创建并初始化线程池
    threadpool<http_conn>  * pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    }catch(...){
        LOG_ERROR("%s", "create threadpoll failure");
        exit(-1);
    }
    
    //创建一个数组，用于保存所有的客户端信息
    http_conn  *  users = new http_conn[ MAX_FD ];
    
    //创建 监听 网络 套接字
    int listenfd = socket( PF_INET , SOCK_STREAM , 0);
    if( -1 == listenfd){
        perror("socket");
        delete pool;
        exit(-1);
    }
    //设置端口复用
    int optname = 1;
    int ret = setsockopt( listenfd , SOL_SOCKET , SO_REUSEADDR , &optname , sizeof(optname));
    if( -1 == ret ){
        perror("setsockopt");
        delete pool;
        close(listenfd);
        exit(-1);
    }
    //绑定
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    //inet_pton( PF_INET , "10.100.90.84" , &addr.sin_addr.s_addr);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( port );
    ret = bind( listenfd , (struct sockaddr *)&addr , sizeof(addr));
    if( -1 == ret ){
        perror("bind");
        delete pool;
        close(listenfd);
        exit(-1);
    }
    //监听
    listen(listenfd , 5);
    //创建 epoll 对象、监听事件数组，将监听描述符添加到到 epoll实例当中
    int epollfd = epoll_create(5);
    struct epoll_event event_array[MAX_EVENT_NUMBER];
    addfd_epoll( epollfd , listenfd , false);//监听描述符 不需要注册 EPOLLONESHOT
    http_conn :: My_epollfd = epollfd;


    while( true ){
        int num  = epoll_wait( http_conn :: My_epollfd ,  event_array , MAX_EVENT_NUMBER , -1);//阻塞等待
        if( (num < 0) && errno != EINTR){
            printf("epoll_wait  failure\n");
            delete pool;
            close(listenfd);
            exit(-1);
        }

        //循环遍历事件数组
        for( int i = 0 ; i < num ; i++){
            int sockfd = event_array[i].data.fd;
            if( sockfd == listenfd ){
                //有客户端连接进来
                struct  sockaddr_in  client_addr;
                socklen_t size = sizeof(client_addr);
                int connect_fd = accept( listenfd , (struct sockaddr *)&client_addr , &size);
                if ( connect_fd < 0 ) {//如果连接失败
                    printf( "errno is: %d\n", errno );
                    continue;
                } 
                if( http_conn::My_users_count >= MAX_FD){
                    //目标连接数已满
                    //给客户端发送信息：服务器正忙
                    close(connect_fd);
                    continue;
                }
                //保存客户端地址信息到日志中
                char ip[16] = {0};
                inet_ntop(AF_INET, &client_addr.sin_addr ,ip, sizeof(ip));
                LOG_INFO("client(%s) is connected", ip);
                //将 新连接的客户信息初始化，放到数组中
                //使用文件描述符作为数组索引
                users[connect_fd].init( connect_fd , client_addr);
            }
            else if(event_array[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR )){
                //客户端异常断开或者错误等事件2222
                users[sockfd].close_connect();
            }
            else if( event_array[i].events & EPOLLIN){
                if(users[sockfd].read()){
                    //一次性把所有数据都读完
                    pool->append( users + sockfd );
                }
                else{
                    users[sockfd].close_connect();
                }
            }
            else if( event_array[i].events & EPOLLOUT ){
                //判断写数据是否成功
                if( !users[sockfd].write() ){
                    users[sockfd].close_connect();
                }
            }
        }
    }
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete [] pool;
    return 0; */
}