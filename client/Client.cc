#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

using namespace mylib;
Client::Client() : neter_(this), userService_(&neter_)
{

    msgHandlerMap_[REG_MSG_ACK] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
    { this->reg_ack(conn, js, time); };
}

void Client::start()
{
    neter_.start();
    std::string email = "1934613109@qq.com";
    std::string password = "1234567890";
    std::string nickname = "winkwink";
    userService_.regiSter(email, password, nickname);
}

void Client::handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data, time);
}

void Client::reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO("reg_ack");
}
