#pragma once

#include <mutex>
#include <condition_variable>
#include "InetAddress.h"
#include "TcpClient.h"
#include "EventLoopThread.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class Client;
class Neter
{
public:
    Neter(Client *client, const char *serverAddr) : client_(client), serverAddr_(serverAddr, 8000) {}
    void start();
    void onMessage(const mylib::TcpConnectionPtr &conn, mylib::Buffer *buf, mylib::Timestamp time);
    void onConnection(const mylib::TcpConnectionPtr &conn);
    void sendJson(json &js);
    mylib::TcpConnectionPtr conn_;

private:
    Client *client_;

    mylib::InetAddress serverAddr_;
    mylib::EventLoopThread loopThread_;
    mylib::EventLoop *loop_;

    std::mutex connMutex_;
    std::condition_variable connCond_;

    std::unique_ptr<mylib::TcpClient> Tcpclient_;
};