1.实现文件锁定。其中最简单的方法就是以原子操作的方式创建锁文件
就是在创建锁文件时，系统将不允许任何其他的事情发生。
这就给程序提供了一种方式来确保它所创建的文件是唯一的，
而且这个文件不可能被其他程序在同一时刻创建

2.用创建锁文件的方法来控制对诸如串行口或不经常访问的文件之类的资源的独占式访问，
是一个不错的选择，但它并不适用于访问大型的共享文件。

文件中的某个特定部分被锁定了，但其他程序可以访问这个文件中的其他部分。这被称为文件段锁定或文件区域锁定


3.文件锁的竞争
    一个文件的某个区域允许多个进程设置共享锁
    对没有上锁的区域进行解锁，也是可以的，因为解锁的最终结果就是让某个区域没有锁
    对于已经有独占锁的文件区域，无法创建共享锁

4.死锁
    在讨论锁定时如果未提到死锁的危险，那么这个讨论就不能算是完整的。
    假设两个程序想要更新同一个文件。它们需要同时更新文件中的字节1和字节2。
    程序A选择首先更新字节2，然后再更新字节1。程序B则是先更新字节1，然后才是字节2。
    两个程序同时启动。程序A锁定字节2，而程序B锁定字节1。然后程序A尝试锁定字节1，
    但因为这个字节已经被程序B锁定，所以程序A将在那里等待。接着程序B尝试锁定字节2，
    但因为这个字节已经被程序A锁定，所以程序B也将在那里等待。
    这种两个程序都无法继续执行下去的情况，就被称为死锁（deadlock或deadly embrace）。