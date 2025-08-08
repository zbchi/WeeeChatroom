#pragma once
#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include "base.h"

#include "Handler.h"
#include "ThreadPool.h"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

#include "File.h"
using json = nlohmann::json;

using OnlineUsers = std::unordered_map<std::string, TcpConnectionPtr>;
class Service
{
public:
    Service();
    void start();

    OnlineUsers onlineUsers_;
    std::mutex onlienUsersMutex_;
    TcpConnectionPtr getConnectionPtr(std::string user_id);
    std::string getUserid(const TcpConnectionPtr &conn);
    void setNumThreads(int numThreads);

private:
    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;
    void handleMessage(const mylib::TcpConnectionPtr &conn, const std::string &jsonStr, mylib::Timestamp time);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void onConnection(const TcpConnectionPtr &conn);

    void heartBeatCheck();
    void insertFromRedis();
    void handleHeartBeat(const TcpConnectionPtr &conn, Timestamp time);

    void initCache();

    ThreadPool threadPool_;
    ThreadPool chatThreadPool_;
    InetAddress listenAddr_;
    EventLoop loop_;
    TcpServer server_;
};

struct HeartBeatContext
{
    Timestamp lastCheckTime;
    HeartBeatContext(Timestamp time) : lastCheckTime(time) {}
};
