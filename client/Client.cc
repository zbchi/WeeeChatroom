#include "Client.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

Client::Client()
{
    msgHandlerMap_[REG_MSG_ACK] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time) {}
}

void Client::handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data, time);
}
void Client::reg(const TcpConnectionPtr &conn)
{
    json regInfo;
    regInfo["msgid"] = REG_MSG;
    regInfo["email"] = "1934613109@qq.com";
    regInfo["password"] = "1234567890";

    std::string msg = regInfo.dump();
    conn->send(msg);

    int code;
    std::cin >> code;
    
}

void Client::parse()
{
}