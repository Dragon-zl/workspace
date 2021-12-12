/*
    #include <sys/ipc.h>
    #include <sys/shm.h>

    int shmget(key_t key, size_t size, int shmflg);
        功能：
            创建一个新的共享内存段 或 获取一个已存在的共享内存段 ，返回共享内存标识符
        参数：
            key：key_t 类型是一个整形，
                    一般为 16进制 ，且非0
            size：共享内存的大小
            shmflg：属性
                - 访问权限
                - 附加属性：创建/判断共享内存是否存在
                    创建：IPC_CREAT
                    判断共享内存是否存在：IPC_EXCL (需要和IPC_CREAT一起使用)
        返回值：
            失败：-1
            成功：>0 返回共享内存引用的ID，后面操作共享内存都通过这个值
    
    
    void *shmat(int shmid, const void *shmaddr, int shmflg);
        功能：设置共享内存段成为调用进程的虚拟内存的一部分，返回值为共享内存段的首地址
        参数：
            shmid：共享内存的ID（标识符）
            shmaddr：指定申请的共享内存的起始地址（一般填 NULL，由内核指定）
            shmflg：对共享内存的操作
                -读：SHM_RDONLY ， 必须要有的权限
                -读写：0
            返回值：
                成功：返回共享内存的首地址
                失败：返回(void *)-1

    int shmdt(const void *shmaddr);
        功能：分离共享内存段，该函数是可选的，进程结束时会自动调用该函数
        参数：
            shmaddr：共享内存首地址
        返回值：
            成功：0
            失败：-1
    
    int shmctl(int shmid, int cmd, struct shmid_ds *buf);
        功能：操作共享内存段，只有当所有附加内存端的进程与之分离之后，才会销毁，只有一个进程需要执行这一步。
        参数：
            shmid：共享内存的ID（标识符）
            cmd：要做的操作
                IPC_STAT：获取共享内存当前的状态
                IPC_SET：设置共享内存的状态
                IPC_RMID：标记共享内存被销毁
            buf：
                需要设置或者获取的共享内存的属性信息
                当cmd为：
                    IPC_STAT：buf为传出参数
                    IPC_SET：buf为传入参数
                    IPC_RMID：没有用 NULL

    key_t ftok(const char *pathname, int proj_id);
        功能：根据指定的路径名，和int值，生成一个共享内存的key
        参数：
            - pathname：指定一个存在的路径
                /home/dzl/workspace
                /
            - proj_id: 一个字节的值
                返回 0~255 ， 一般指定 一个字符 'a'
*/
