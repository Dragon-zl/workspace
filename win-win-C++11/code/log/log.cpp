/*
 * @Author       : The Lin
 * @Date         : 2022-02-25
 */ 
#include "log.h"

using namespace std;

Log::Log() {
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    deque_ = nullptr;
    toDay_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if(writeThread_ && writeThread_->joinable()) {
        while(!deque_->empty()) {
            deque_->flush();
        };
        deque_->Close();
        writeThread_->join();
    }
    if(fp_) {
        lock_guard<mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level = 1, const char* path, const char* suffix,
    int maxQueueSize) {
    isOpen_ = true;
    level_ = level;
    if(maxQueueSize > 0) {
        /*异步日志模式*/
        isAsync_ = true;
        if(!deque_) {
            /*unique_ptr 是 C++ 11 提供的用于防止内存泄漏的智能指针中的一种实现，
                独享被管理对象指针所有权的智能指针。*/
            unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            
            /*C++ 11 :将一个左值强制转化为右值引用*/
            deque_ = move(newDeque);
            
            /*new 线程池对象*/
            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));

            writeThread_ = move(NewThread);
        }
    } else {
        isAsync_ = false;
    }

    lineCount_ = 0;
    /*获取当前时间*/
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    /*用当前时间 格式化 log 的文件名*/
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll();
        if(fp_) { 
            flush();
            fclose(fp_); 
        }

        fp_ = fopen(fileName, "a");
        if(fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        } 
        assert(fp_ != nullptr);
    }
}
/*日志写：格式化传入参数*/
void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    /*获取当前时间*/
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    /* 日志日期 日志行数 */
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_  %  MAX_LINES == 0)))
    {
        /*lock_guard<mutex>:
        C++11 锁管理：构造函数加锁，析构函数解锁，大大降低了死锁的风险*/
        unique_lock<mutex> locker(mtx_);
        /*先解锁*/
        locker.unlock();
        
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        /*格式化新的log文件名*/
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }
        /*上锁*/
        locker.lock();
        /*刷新文件描述符区*/
        flush();
        /*关闭之前的文件描述符*/
        fclose(fp_);
        /*打开新的文件描述符*/
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    {
        unique_lock<mutex> locker(mtx_);
        /*日志行数++*/
        lineCount_++;
        /*格式化需要写入的log信息---时间*/
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        /*写索引右移*/
        buff_.HasWritten(n);
        /*将log写入状态赋值到buff_*/
        AppendLogLevelTitle_(level);

        va_start(vaList, format);
        /*格式化需要写入的log信息---具体信息*/
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);
        /*写索引右移*/
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        /*异步写日志*/
        if(isAsync_ && deque_ && !deque_->full()) {
            /*将数据入队*/
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle_(int level) {
    switch(level) {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}
/*刷新文件描述符的缓冲区*/
void Log::flush() {
    if(isAsync_) { 
        deque_->flush(); 
    }
    fflush(fp_);
}

/*异步日志：循环等待阻塞队列出队后写入*/
void Log::AsyncWrite_() {
    string str = "";
    while(deque_->pop(str)) {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}