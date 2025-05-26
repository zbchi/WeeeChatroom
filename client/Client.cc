#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

using namespace mylib;
Client::Client() : neter_(this), userService_(&neter_, this),
                   chatService_(&neter_, this),
                   controller_(&neter_, this)
{
    msgHandlerMap_[CHAT_MSG] =
        [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleMessage(conn, js); };
}

void Client::start()
{
    neter_.start();
    controller_.mainLoop();
}

void Client::handleJson(const TcpConnectionPtr &conn, std::string &jsonStr)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}
