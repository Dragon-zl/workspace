# config 类

## 说明

```c++
config(参数)类
1、用于个性化运行时的OS终端输入的参数解析
2、成员变量保存了各个模块的技术实现标志位，和相应的参数
```

## 核心实现方式

```c++
核心解析终端输入参数的函数 void Config::parse_arg(int argc, char *argv[])
1、调用了 getopt 函数，对终端输入参数逐个遍历
```

