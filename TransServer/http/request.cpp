#include "request.h"
using namespace std;

void Request::Init() {
    userName_ = operator_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
}

bool Request::IsKeepAlive() const {
//    if(header_.count("Connection") == 1) {
//        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
//    }
    return true;
}

bool Request::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    while(buff.ReadableBytes() && state_ != FINISH) {
        // 获取一行数据，以\r\n为结束标志
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);
        switch(state_)
        {
        case REQUEST_LINE:  // 请求行，用来说明请求类型以及所使用的HTTP版本
            ParseRequestLine_(line);
            break;    
        case HEADERS:  // 请求头部
            ParseHeader_(line);
            if(buff.ReadableBytes() <= 0) {
                state_ = FINISH;
            }
            break;
        case BODY:  // 请求数据也叫主体，可以添加任意的其他数据
            ParseBody_(line);
            break;
        default:
            break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
    return true;
}

bool Request::ParseRequestLine_(const string& line) {
    userName_ = line;
    cout << "userName_: " << userName_ << endl;
    state_ = HEADERS;
    return true;
}

void Request::ParseHeader_(const string& line) {
    operator_ = line;
    cout << "operator_: " << operator_ << endl;
    state_ = BODY;
}

void Request::ParseBody_(const string& line) {
    body_ = line;
    cout << "body_: " << body_ << endl << endl;
    state_ = FINISH;
}
