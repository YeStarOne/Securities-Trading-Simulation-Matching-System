#include <unistd.h>
#include "server/TransServer.h"

int main() {
    TransServer server(
        1316, 3, 120000, false,                 /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "123", "SecTransDb",       /* Mysql配置 */
        12, 6, true, 0, 1024); /* sql连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
} 
  