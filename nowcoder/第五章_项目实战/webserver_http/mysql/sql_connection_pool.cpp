#include "sql_connection_pool.h"

//构造函数
connection_pool :: connection_pool(){
    //当前使用连接数，置零
    m_CurConn = 0;
    //空闲数，      置零
    m_FreeConn = 0;
}

//析构函数
connection_pool :: ~connection_pool(){
    //销毁数据库连接池
    DestoryPool();
}

//单例模式
connection_pool * connection_pool :: GetInstance(){
    static connection_pool connPool;
    return &connPool;
}

//销毁所有连接
void connection_pool :: DestoryPool(){
    //上锁
    lock.lock();
    //判断 链表中 是否有数据
    if(connList.size() > 0){
        //创建 迭代器
        list<MYSQL*> :: iterator it;
        //遍历 链表
        for(it = connList.begin() ; it != connList.end() ; ++it){
            MYSQL * con = *it;
            //关闭销毁数据库
            mysql_close(con);
        }
        //当前使用连接数，置零
        m_CurConn = 0;
        //空闲数，      置零
        m_FreeConn = 0;
        //清空链表
        connList.clear();
    }
    //解锁
    lock.unlock();
}

//数据库初始化函数
void connection_pool :: init(string url , string User , string PassWord  ,
                    string DatabaseName , int Port , int MaxConn , int close_log){
    
    //依次给成员变量 赋值
    m_url = url;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DatabaseName;
    m_Port = Port;
    m_MaxConn = MaxConn;
    m_close_log = close_log;

    //创建 m_MaxConn 个 数据库对象 进行入队
    for(int i = 0 ; i < m_MaxConn ; i++){
        MYSQL * con = NULL;
        con = mysql_init(con);

        //判断是否初始化失败
        if(con == NULL){
            LOG_ERROR("%s", "MySQL error");
            exit(1);
        }
        //初始化成功
        //入队
        connList.push_back(con);
        //空闲个数 +1
        ++m_FreeConn;
    }
    //创建 m_FreeConn 个信号
    reserve = sem(m_FreeConn);

    m_MaxConn = m_FreeConn;
}

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL * connection_pool :: GetConnection(){
    MYSQL * con = NULL;
    //判断链表是否为空
    if( 0 == connList.size()){
        return NULL;
    }
    //信号量阻塞,等待
    reserve.wait();
    //上锁
    lock.lock();
    //获取链表第一个节点
    con = connList.front();
    //链表第一个节点出队
    connList.pop_front();

    //空闲数 减一 ， 使用数 加一
    --m_FreeConn;
    ++m_CurConn;

    lock.unlock();
    return con;
}

//释放当前使用的连接
bool connection_pool :: ReleaseConnection(MYSQL * con){
    if( NULL == con){
        return false;
    }
    //上锁
    lock.lock();
    //将当前使用的连接 归还(入队)
    connList.push_back(con);

    //空闲数 加一  使用数 减一
    ++m_FreeConn;
    --m_CurConn;

    lock.unlock();

    //增加信号量，唤醒
    reserve.post();
    return true;
}

//获取当前空闲的连接数
int connection_pool :: GetFreeConn(){
    return this -> m_FreeConn;
}


connectionRAII :: connectionRAII(MYSQL **SQL , connection_pool * connPool){
    *SQL = connPool -> GetConnection();

    conRAII = *SQL;
    poolRAII = connPool;
}
connectionRAII :: ~connectionRAII(){
    poolRAII -> ReleaseConnection(conRAII);
}