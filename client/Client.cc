#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

using namespace mylib;
Client::Client() : neter_(this), controller_(&neter_, this),
                   chatService_(&neter_, this),
                   userService_(&neter_, this),
                   friendService_(&neter_, this)
{

    msgHandlerMap_[LOGIN_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->userService_.handleLoginAck(conn, js); };
    msgHandlerMap_[REG_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->userService_.handleRegAck(conn, js); };
    msgHandlerMap_[GET_FRIENDS] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleFriendsList(conn, js); };
    msgHandlerMap_[CHAT_MSG] = [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleMessage(conn, js); };
}

void Client::start()
{
    // 网络接收线程将消息放入消息队列
    neter_.start();
    // 逻辑处理线程负责处理消息更新内存
    logicThread_ = std::thread([this]()
                               { this->logicLoop(); });

    // 主线程负责主控制输出UI
    controller_.mainLoop();
}

void Client::logicLoop()
{
    while (1)
    {
        json js = messageQueue_.pop();
        std::string jsonStr = js.dump();
        handleJson(neter_.conn_, jsonStr);
    }
}

void Client::handleJson(const TcpConnectionPtr &conn, const std::string &jsonStr)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}
