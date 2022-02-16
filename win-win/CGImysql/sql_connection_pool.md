# sql_connection_pool

数据库连接池

## 说明

```c++
1、数据库连接池
    程序一开始运行，即分配一个链表，使用 mysql_init 和 mysql_real_connect 函数，创建指定个数的数据库连接对象，然后将指定个数的数据库连接对象插入链表
2、客户端连接时
    有客户端连接时，即 取出链表中一个数据库连接对象，供其使用
3、客户端断开时
    将正在使用的数据量连接对象，重新插入链表中
```

## 核心函数

```c++
MYSQL * con = NULL;
//分配初始化与mysql_real_connect()相适应的MySQL对象
con = mysql_init(con);
//连接数据库引擎
con = mysql_real_connect(con, ip.c_str(), User.c_str(),
                         PassWord.c_str(), DBName.c_str(), Port, NULL, 0);
```

