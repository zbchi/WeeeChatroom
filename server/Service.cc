#include <string>
#include <iostream>
#include <ctime>

#include "Service.h"
#include "Register.h"
#include "Loginer.h"

#include "base.h"
#include <arpa/inet.h>

Service::Service() : threadPool_(16)
{
    handlers_[REG_MSG] = std::make_shared<Register>(this);
    handlers_[REG_MSG_ACK] = std::make_shared<RegisterAcker>(this);
    handlers_[LOGIN_MSG] = std::make_shared<Loginer>(this);
}

void Service::handleMessage(const mylib::TcpConnectionPtr &conn,
                            const std::string &jsonStr, mylib::Timestamp time)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = handlers_.find(msgid);
    if (it != handlers_.end())
        it->second->handle(conn, data, time);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}