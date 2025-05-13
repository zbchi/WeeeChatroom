#include "Client.h"
#include "base.h"
#include <iostream>

Client::Client()
{

    msgHandlerMap_[REG_MSG_ACK] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
    { this->reg_ack(conn, js, time); };
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
    regInfo["nickname"] = "winkwink";
    regInfo["password"] = "1234567890";

    sendJson(conn, regInfo);

    int code;
    std::cin >> code;
    regInfo["msgid"] = REG_MSG_ACK;
    regInfo["code"] = code;
    sendJson(conn, regInfo);
}

void Client::reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO("reg_ack");
}

void Client::parse()
{
}