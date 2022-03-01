#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

#include <arpa/inet.h>
#include <map>

class TcpClient
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        DEFAULT = 0,//默认状态
        GET,//获取
        POST    //上传
    };

public:
    TcpClient(){};
    ~TcpClient(){};
public:
    void init(int sockfd, const sockaddr_in &addr, char *,
            int, int, string user, string passwd, string sqlname);
    void close_client(bool read_close = true);
    void process();
    bool ClientContinuSendCMD(int sockfd);
    bool read_once();
    bool m_write();
    void GetClientState();
    sockaddr_in * get_address(){
        return &m_address;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state; //读为0, 写为1
private:
    void init();
	void unmap();
    void PostState(string &str_recvdata);
    void GETState(string &str_recvdata);
    bool CGIMysqlInertLine(string ClientData);
    bool CGIMysqlQueryLine(string& barcode);
    bool CGIMysqlUPDATERows(string &sql_UPDATE); 
    bool MySQL_COMMIT(MYSQL *mysql);
    char *m_fileaddress;
    int m_write_idx;
	int write_file_size;
	bool StateVerifyCMD(int sockfd,const char * cmd);
    void ClientPOST();
    void ClientGET();
	int m_sockfd;
    int m_read_idx;//读取数据时：当前指针索引
    sockaddr_in m_address;
    FILE * fd;
    int m_TRIGMode;
    map<string, string> m_users;
    char *doc_root;
    int m_close_log;
    bool m_linger;
    METHOD m_method;
	struct stat m_file_stat;
    int bytes_to_send;
    int bytes_have_send;
    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
    char m_read_buf[READ_BUFFER_SIZE];
    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];
};

#endif
