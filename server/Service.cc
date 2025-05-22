#include <string>
#include <iostream>
#include <ctime>

#include "Service.h"
#include "Register.h"
#include "Loginer.h"
// #include "Adder.h"

#include "base.h"
#include <arpa/inet.h>

Service::Service() : threadPool_(16),
                     listenAddr_(8000),
                     server_(&loop_, listenAddr_)
{
    handlers_[REG_MSG] = std::make_shared<Register>(this);
    handlers_[REG_MSG_ACK] = std::make_shared<RegisterAcker>(this);
    handlers_[LOGIN_MSG] = std::make_shared<Loginer>(this);
    // handlers_[ADD_FRIEND] = std::make_shared<AdderFriend>(this);
    // handlers_[ADD_GROUP] = std::make_shared<AdderGroup>(this);

    server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                  { this->onConnection(conn); });
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                               { this->onMessage(conn, buf, time); });
}

void Service::start()
{
    server_.start();
    loop_.loop();
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

void Service::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
    }
    else
    {
        std::lock_guard<std::mutex> lock(onlienUsersMutex_);
        for (auto it = onlienUsers_.begin(); it != onlienUsers_.end(); it++)
        {
            if (it->second == conn)
            {
                onlienUsers_.erase(it);
                break;
            }
        }
    }
}

TcpConnectionPtr Service::getConnectionPtr(std::string user_id)
{
    std::lock_guard<std::mutex> lock(onlienUsersMutex_);
    auto it = onlienUsers_.find(user_id);
    if (it != onlienUsers_.end())
        return it->second;
    else
        return nullptr;
}

void Service::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    while (buf->readableBytes() >= 4)
    {
        const void *data = buf->peek();
        int lenNetOrder;
        memcpy(&lenNetOrder, data, sizeof(lenNetOrder));
        int len = ntohl(lenNetOrder);
        if (buf->readableBytes() < 4 + len)
            break;
        buf->retrieve(4);

        std::string jsonStr(buf->peek(), len);
        buf->retrieve(len);
        std::cout << jsonStr << std::endl;

        threadPool_.add_task([conn, jsonStr, time, this]()
                             { handleMessage(conn, jsonStr, time); });
    }
}