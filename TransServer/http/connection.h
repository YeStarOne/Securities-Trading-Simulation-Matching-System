#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include <set>

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "request.h"
#include "response.h"

class Connection {
public:
    Connection();

    ~Connection();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;
    
    bool process();

    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    auto formatDoubleValue(double val, int fixed);

    bool delOperator();
    bool sendSupportSec();
    bool delBuyOrSell();
    bool delRevoke();
    bool delUserPublish();

    static bool isET;  // 是否是 ET 模式
    static std::atomic<int> userCount; // 当前连接的客户端的数量
    
private:
   
    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;
    
    int iovCnt_;
    struct iovec iov_[2];  // 响应报文  iov[0]:响应头  iov[1]:响应体
    
    Buffer readBuff_;  // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    Request request_;
    Response response_;

    std::vector<std::vector<std::string>> allowRevoke_;
};


#endif //CONNECTION_H