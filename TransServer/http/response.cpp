#include "response.h"

using namespace std;

Response::Response() {
    isKeepAlive_ = false;
};

Response::~Response() = default;

void Response::Init(bool isKeepAlive){
    isKeepAlive_ = isKeepAlive;
}

void Response::MakeResponse(Buffer& buff) {



    buff.Append("Content-length: \r\n\r\n");
}
