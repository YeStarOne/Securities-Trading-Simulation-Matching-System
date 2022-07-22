#include "connection.h"
using namespace std;

std::atomic<int> Connection::userCount;
bool Connection::isET;

std::vector<std::string> securityName{"银行600000", "银行600001", "银行600002", "银行600003", "银行600004",
                                      "银行600005", "银行600006", "银行600007", "银行600008", "银行600009",
                                      "银行700000", "银行700001", "银行700002", "银行700003", "银行700004",
                                      "银行700005", "银行700006", "银行700007", "银行700008", "银行700009"};

Connection::Connection() {
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
}

Connection::~Connection() {
    Close(); 
}

void Connection::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount)
}

void Connection::Close() {
    if(!isClose_){
        isClose_ = true; 
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount)
    }
}

int Connection::GetFd() const {
    return fd_;
}

struct sockaddr_in Connection::GetAddr() const {
    return addr_;
}

const char* Connection::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int Connection::GetPort() const {
    return addr_.sin_port;
}

ssize_t Connection::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t Connection::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        // 将响应报文写入tcp发送缓冲区
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {  // 有响应体
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {  // 没有响应体
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}

bool Connection::process() {
    request_.Init();
    // 判断缓冲区是否有未被读取的数据
    if(readBuff_.ReadableBytes() <= 0) {
        return false;
    }
    else if(request_.parse(readBuff_)) {
        response_.Init(request_.IsKeepAlive());
    } else {
        response_.Init(false);
    }

    delOperator();

    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    return true;
}

bool Connection::delOperator() {
    LOG_DEBUG( "[user:%s] [operator:%s]", request_.userName_.c_str(), request_.operator_.c_str())
    if(request_.operator_ == "getSupportSec") {
        sendSupportSec();
    }
    else if(request_.operator_ == "buy" || request_.operator_ == "sell") {
        delBuyOrSell();
    }
    else if(request_.operator_ == "revoke") {
        delRevoke();
    }
    else if(request_.operator_ == "userPublish") {
        delUserPublish();
    }
    else {
        return false;
    }
    return true;
}

// 浮点数转换为字符串，保留 fixed 个小数位
auto Connection::formatDoubleValue(double val, int fixed) {
    auto str = std::to_string(val);
    return str.substr(0, str.find(".") + fixed + 1);
}


bool Connection::delBuyOrSell() {
    if(request_.body_.empty()) {
        return false;
    }

    string str;
    vector<string> str_vec;

    istringstream istr(request_.body_);
    while(getline(istr, str, ';')) {
        str_vec.push_back(str);
    }
    if(str_vec.size() != 3) {
        return false;
    }

    string secName = str_vec[0];
    float price_1 = strtof(str_vec[1].c_str(), nullptr);
    int quantity_1 = atoi(str_vec[2].c_str());
    // 请求方的交易类型
    string type_1 = request_.operator_;
    string type_2, type_3, type_4;
    if(type_1 == "buy") {
        type_2 = "sell";
        type_3 = "B(内盘)";
        type_4 = "买入";
    }
    else if(type_1 == "sell") {
        type_2 = "buy";
        type_3 = "S(外盘)";
        type_4 = "卖出";
    }

    MYSQL* sql = nullptr;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);
    MYSQL_FIELD *fields = nullptr;

    /* 查询 */
    char order[256] = { 0 };
    if(type_1 == "buy") {
        snprintf(order, 256,"SELECT * FROM transOrder WHERE trans_type='%s' AND sec_name='%s' "
                 "AND %f-price>'-1e-3' ORDER BY price, date;", type_2.c_str(), secName.c_str(), price_1);
    }
    else {
        snprintf(order, 256,"SELECT * FROM transOrder WHERE trans_type='%s' AND sec_name='%s' "
                 "AND price-%f>'-1e-3' ORDER BY price DESC, date;", type_2.c_str(), secName.c_str(), price_1);
    }
    // 如果查询成功，返回0。如果出现错误，返回非0值
    LOG_DEBUG( "%s", order)
    if(mysql_query(sql, order)) {
        LOG_DEBUG( "Sql order error!")
    }

    // 读取查询的全部结果，分配1个MYSQL_RES结构，并将结果置于该结构中
    MYSQL_RES *res = mysql_store_result(sql);
    // mysql_fetch_row() 获取多行结果的一行内容
    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("%s %s %s %s %s %s", row[5], row[0], row[1], row[2], row[3], row[4])
        int quantity_2 = atoi(row[4]);
        float price_2 = strtof(row[3], nullptr);
        string user_name = row[0];

        writeBuff_.Append("[成交] [用户 " + user_name + "] [" + formatDoubleValue(price_2, 2) + "元] [" +
                          to_string(min(quantity_1, quantity_2)) + "股] [" + type_3 + "]\n");

        bzero(order, 256);
        // 想买入/卖出的比能卖出/买入的多
        if(quantity_1 >= quantity_2) {
            quantity_1 -= quantity_2;
            // 请求方全部买入/卖出
            // 移除历史订单
            snprintf(order, 256,"DELETE FROM transOrder WHERE user_name='%s' AND trans_type='%s' AND "
                                "sec_name='%s' AND ABS(price-%f)<'1e-5' AND quantity='%d';", row[0], row[1], row[2], price_2, quantity_2);
        }
        else {
            // 买入/卖出需求全部满足
            quantity_2 -= quantity_1;
            quantity_1 = 0;
            bzero(order, 256);
            snprintf(order, 256,"UPDATE transOrder SET quantity='%d' WHERE user_name='%s' AND trans_type='%s' "
                                "AND sec_name='%s' AND ABS(price-%f)<'1e-5';", quantity_2, row[0], row[1], row[2], price_2);
        }
        LOG_DEBUG( "%s", order)
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Sql order error!")
        }

        if(quantity_1 == 0) {
            break;
        }
    }

    mysql_free_result(res);
    // 需求没有被全部满足，留待后续撮合
    if(quantity_1 > 0) {
        // 增加买入订单
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO transOrder(user_name, trans_type, sec_name, price, quantity) "
                 "VALUES ('%s','%s','%s','%f','%d');", request_.userName_.c_str(), type_1.c_str(), secName.c_str(), price_1, quantity_1);
        LOG_DEBUG( "%s", order)
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Sql order error!")
        }
        writeBuff_.Append("加入订单: [" + type_4 + "] [" + secName + "] [" + formatDoubleValue(price_1, 2) + "元] [" + to_string(quantity_1) + "股]\n");
    }

    SqlConnPool::Instance()->FreeConn(sql);
    return true;
}

bool Connection::delRevoke() {
    string str;
    vector<string> str_vec;

    istringstream istr(request_.body_);
    while(getline(istr, str, ';')) {
        str_vec.push_back(str);
    }

    char order[256] = { 0 };
    MYSQL* sql = nullptr;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);

    if(str_vec.size() == 1 && str_vec[0] == "start") {
        bool flag = false;
        allowRevoke_.clear();
        snprintf(order, 256,"SELECT * FROM transOrder WHERE user_name='%s';", request_.userName_.c_str());
        // 如果查询成功，返回0。如果出现错误，返回非0值
        LOG_DEBUG( "%s", order)
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Sql order error!")
        }

        // 读取查询的全部结果，分配1个MYSQL_RES结构，并将结果置于该结构中
        MYSQL_RES *res = mysql_store_result(sql);
        int cnt = 0;
        while(MYSQL_ROW row = mysql_fetch_row(res)) {
            LOG_DEBUG("%s %s %s %s %s %s", row[5], row[0], row[1], row[2], row[3], row[4])
            allowRevoke_.push_back(vector<string>(6));
            for(int i = 0; i < 6; ++i) {
                allowRevoke_.back()[i] = row[i];
            }
            float revPrice = strtof(allowRevoke_.back()[3].c_str(), nullptr);
            writeBuff_.Append("[订单号:" + to_string(cnt++) + "] ");
            writeBuff_.Append("[" + allowRevoke_.back()[5] + "] ");
            writeBuff_.Append("[" + allowRevoke_.back()[1] + "] ");
            writeBuff_.Append("[" + allowRevoke_.back()[2] + "] ");
            writeBuff_.Append("[" + formatDoubleValue(revPrice, 2) + "元] ");
            writeBuff_.Append("[" + allowRevoke_.back()[4] + "股];");
            flag = true;
        }
        mysql_free_result(res);
        if(!flag) {
            writeBuff_.Append("#");
        }
    }
    else {
        writeBuff_.Append("撤销结果如下:\n");
        cout << request_.body_;
        for(auto& revOrder : str_vec) {
            istringstream istr1(revOrder);
            getline(istr1, str, ',');
            int index = atoi(str.c_str());
            getline(istr1, str, ';');
            int quantity = atoi(str.c_str());

            bzero(order, 256);
            snprintf(order, 256,"SELECT * FROM transOrder WHERE date='%s' AND user_name='%s';",
                     allowRevoke_[index][5].c_str(), request_.userName_.c_str());

            // 如果查询成功，返回0;如果出现错误，返回非0值
            if(mysql_query(sql, order)) {
                writeBuff_.Append("[" + to_string(index) + "]号订单已经成交了,无法撤销\n");
                continue;
            }

            // 读取查询的全部结果，分配1个MYSQL_RES结构，并将结果置于该结构中
            MYSQL_RES *res = mysql_store_result(sql);

            while(MYSQL_ROW row = mysql_fetch_row(res)) {
                LOG_DEBUG("%s %s %s %s %s %s", row[5], row[0], row[1], row[2], row[3], row[4])
                int revokeQua = 0;
                float price = strtof(row[3], nullptr);
                bzero(order, 256);
                if(quantity < atoi(row[4])) {
                    revokeQua = quantity;
                    quantity = atoi(row[4]) - quantity;
                    snprintf(order, 256,"UPDATE transOrder SET quantity='%d' WHERE user_name='%s' AND trans_type='%s' "
                                        "AND sec_name='%s' AND ABS(price-%f)<'1e-5' AND date='%s';", quantity, row[0], row[1], row[2], price, row[5]);
                }
                else {
                    revokeQua = atoi(row[4]);
                    snprintf(order, 256,"DELETE FROM transOrder WHERE user_name='%s' AND trans_type='%s' AND "
                                        "sec_name='%s' AND ABS(price-%f)<'1e-5' AND quantity='%d' AND date='%s';", row[0], row[1], row[2], price, revokeQua, row[5]);
                }
                LOG_DEBUG( "%s", order)
                if(mysql_query(sql, order)) {
                    LOG_DEBUG( "Sql order error!")
                }
                writeBuff_.Append("[" + to_string(index) + "]号订单成功撤销了[" + to_string(revokeQua) + "股]\n");
            }
            mysql_free_result(res);
        }
    }

    SqlConnPool::Instance()->FreeConn(sql);

    return true;
}

bool Connection::delUserPublish() {
    bool flag = false;
    string str;
    vector<string> str_vec;

    istringstream istr(request_.body_);
    while(getline(istr, str, ';')) {
        str_vec.push_back(str);
    }

    char order[256] = { 0 };
    MYSQL* sql = nullptr;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);

    for(auto& secName : str_vec) {
        flag = true;
        writeBuff_.Append("[" + secName + "]:\n");
        snprintf(order, 256,"SELECT * FROM transOrder WHERE sec_name='%s' "
                            "ORDER BY trans_type, price, quantity;", secName.c_str());
        LOG_DEBUG( "%s", order)
        if(mysql_query(sql, order)) {
            LOG_DEBUG( "Sql order error!")
        }
        MYSQL_RES *res = mysql_store_result(sql);

        while(MYSQL_ROW row = mysql_fetch_row(res)) {
            float pubPrice = strtof(string(row[3]).c_str(), nullptr);
            string type = string(row[1]);
            if(type == "buy") {
                type = "买入";
            }
            else {
                type = "卖出";
            }
            writeBuff_.Append("[" + type + "] [" + formatDoubleValue(pubPrice, 2) + "元] [" + string(row[4]) + "股]\n");
        }
        mysql_free_result(res);
    }

    SqlConnPool::Instance()->FreeConn(sql);

    if(!flag) {
        writeBuff_.Append("#");
    }

    return true;
}

bool Connection::sendSupportSec() {
    bool flag = false;
    for(auto& s : securityName) {
        writeBuff_.Append(s + ";");
        flag = true;
    }
    if(!flag) {
        writeBuff_.Append("#");
    }
    return true;
}
