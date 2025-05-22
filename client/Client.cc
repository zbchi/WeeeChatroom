#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

using namespace mylib;
Client::Client() : neter_(this), userService_(&neter_, this),
                   chatService_(&neter_),
                   controller_(this, &neter_)
{
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
