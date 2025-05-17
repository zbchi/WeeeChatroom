#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

#include "Register.h"
using namespace mylib;
Client::Client() : neter_(this), userService_(&neter_), controller_(this, &neter_)
{

    handlers_[REG_MSG_ACK] = std::make_shared<Register>(this);
}

void Client::start()
{
    neter_.start();
    controller_.mainLoop();
}

void Client::handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = handlers_.find(msgid);
    if (it != handlers_.end())
        it->second->handle(conn, data);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}

void Client::reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO("reg_ack");
}
