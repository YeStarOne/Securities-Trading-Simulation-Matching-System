#ifndef RESPONSE_H
#define RESPONSE_H

#include <unordered_map>
#include <sys/stat.h>    // stat

#include "../buffer/buffer.h"
#include "../log/log.h"

class Response {
public:
    Response();
    ~Response();

    void Init(bool isKeepAlive = true);
    void MakeResponse(Buffer& buff);

private:

    bool isKeepAlive_;  //是否保持连接

};


#endif //RESPONSE_H