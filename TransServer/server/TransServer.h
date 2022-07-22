#ifndef TRANSSERVER_H
#define TRANSSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/connection.h"

class TransServer {
public:
    TransServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~TransServer();
    void Start();

private:
    bool InitSocket_(); 
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);
  
    void DealListen_();
    void DealWrite_(Connection* client);
    void DealRead_(Connection* client);

    void SendError_(int fd, const char*info);
    void ExtentTime_(Connection* client);
    void CloseConn_(Connection* client);

    void OnRead_(Connection* client);
    void OnWrite_(Connection* client);
    void OnProcess(Connection* client);

    static const int MAX_FD = 65536;  // 连接文件描述符的最大数量

    static int SetFdNonblock(int fd); // 设置文件描述符非阻塞

    int port_;        // 端口
    bool openLinger_; // 是否打开优雅关闭
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;    // 是否关闭
    int listenFd_;    // 监听的文件描述符
    char* srcDir_;    // 资源的目录
    
    uint32_t listenEvent_; // 监听的文件描述符的事件
    uint32_t connEvent_;   // 连接的文件描述符的事件
   
    std::unique_ptr<HeapTimer> timer_;        // 定时器
    std::unique_ptr<ThreadPool> threadpool_;  // 线程池
    std::unique_ptr<Epoller> epoller_;        // epoll对象
    std::unordered_map<int, Connection> users_; // 客户端连接的信息，键为客户端的文件描述符
};


#endif //TRANSSERVER_H