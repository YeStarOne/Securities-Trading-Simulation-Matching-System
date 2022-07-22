#ifndef REQUEST_H
#define REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class Request {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };
    
    Request() { Init(); }
    ~Request() = default;

    void Init();
    bool parse(Buffer& buff);

    bool IsKeepAlive() const;

    std::string userName_, operator_, body_;    //用户名、操作类型、请求体

private:
    bool ParseRequestLine_(const std::string& line);
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    PARSE_STATE state_; // 解析状态
    std::unordered_map<std::string, std::string> header_;   //请求头

};

#endif //REQUEST_H