#include <gdbm-ndbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  TEST_DB_FILE   "./dbm1_test"
#define  ITEMS_USED     3

typedef  struct
{
    char misc_chars[15];
    int  any_integer;
    char more_chars[21];
}Test_data;

/*
在main函数中，设置了items_to_store和items_received两个结构，还设置了关键字字符串和datum结构
*/
int main()
{
    Test_data  items_to_store[ITEMS_USED];  //数据数组，模拟三个用户数据
    Test_data  item_retrieved;              //用于接收查询到的数据

    char key_to_use[20];
    int i , result;

    datum  key_datum;
    datum  data_datum;

    DMB  *  dbm_ptr;
    // 1. 在声明了一个指向dbm类型结构的指针后，现在打开测试数据库用来读写，如果需要就创建它：
    dbm_ptr = dbm_open( TEST_DB_FILE ,  O_RDWR | O_CREAT , 0666 );
    if( !dbm_ptr ) //失败，它将返回（DBM *）0
    {
        fprintf( stderr , "Failed to open database\n");
        exit(EXIT_FAILURE);
    }
    // 2. 现在添加一些数据到items_to_store结构中
    memset(items_to_store , '\0' , sizeof(items_to_store) );//将数组全部置零

    strcpy( items_to_store[0].misc_chars , "First!" );
    items_to_store[0].any_integer = 47;
    strcpy( items_to_store[0].more_chars , "foo" );

    strcpy( items_to_store[1].misc_chars , "bar" );
    items_to_store[1].any_integer = 13;
    strcpy( items_to_store[1].more_chars , "unlucky?" );

    strcpy( items_to_store[2].misc_chars , "Third" );
    items_to_store[2].any_integer = 3;
    strcpy( items_to_store[2].more_chars , "baz" );
    /* 
    3. 你需要为每个数据项建立一个供以后引用的关键字。
        它被设为每个字符串的头一个字母加上整数。这个关键字由key_datum标识，
        而data_datum则指向items_to_store数据项。然后将数据存储到数据库中
    */
    for( i = 0 ; i < ITEMS_USED ; i++ )
    {
        sprintf( key_to_use , "%c%c%d" , 
                                        items_to_store[i].misc_chars[0],
                                        items_to_store[i].more_chars[0],
                                        items_to_store[i].any_integer);
        key_datum.dptr = (void *)key_to_use;//成员dptr指向数据的起始点
        key_datum.dsize = strlen(key_to_use);//成员dsize设为包含数据的长度

        data_datum.dptr = (void *)&items_to_store[i];
        data_datum.dsize = sizeof(Test_data);

        result = dbm_store( dbm_ptr , key_datum ,data_datum , DBM_REPLACE);//DBM_REPLACE 有则 替换的形式创建
        if( result != 0)
        {
            fprintf(stderr , "dbm_store failed on key %s\n" , key_to_use);
            exit(2);
        }
    }
    /*
    4. 接下来，查看是否可以检索这个新存入的数据。最后，关闭数据库：
    */
    sprintf( key_to_use , "bu%d" , 13);
    key_datum.dptr = key_to_use;
    key_datum.dsize = strlen(key_to_use);
    data_datum = dbm_fetch( dbm_ptr , key_datum );
    if( data_datum.dptr )
    {
        printf("Data  retrieved\n");
        memcpy(&item_retrieved , data_datum.dbm_ptr , data_datum.dsize);
        printf("Retrieved  item - %s %d %s\n" ,
                item_retrieved.misc_chars,
                item_retrieved.any_integer,
                item_retrieved.more_chars);
    }
    else
    {
        printf("No data found for key %s\n" , key_to_use);
    }
    dbm_close(dbm_ptr);
    exit(EXIT_SUCCESS);
}