#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
Client::Client() : serverAddr_("127.0.0.1", 8000)
{

    msgHandlerMap_[REG_MSG_ACK] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
    { this->reg_ack(conn, js, time); };
}

void Client::start()
{
    loop_ = loopThread_.startLoop();
    client_ = std::make_unique<TcpClient>(loop_, serverAddr_);

    client_->setConnectionCallback([this](const TcpConnectionPtr &conn)
                                   { if(conn->connected())
                                {
                                    std::lock_guard<std::mutex> lock(connMutex_);
                                    conn_=conn;
                                }
                                connCond_.notify_one(); });
    client_->setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                { this->onMessage(conn, buf, time); });
    client_->connect();

    // 等待recv线程连接
    {
        std::unique_lock<std::mutex> lock(connMutex_);
        connCond_.wait(lock, [this]
                       { return conn_ != nullptr; });
    }

    // 主线程入口
    reg(conn_);
}

void Client::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
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
        handleMessage(conn, jsonStr, time);
    }
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