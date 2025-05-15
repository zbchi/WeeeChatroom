#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include "InetAddress.h"
#include "TcpClient.h"
#include "EventLoopThread.h"
using namespace mylib;
using json = nlohmann::json;
class Client
{
public:
    Client();
    void start();

    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void send(std::string &str);
    void recieve();
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time);
    void reg(const TcpConnectionPtr &conn);
    void reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void parse();

private:
    InetAddress serverAddr_;
    EventLoopThread loopThread_;
    EventLoop *loop_;

    std::mutex connMutex_;
    std::condition_variable connCond_;
    TcpConnectionPtr conn_;

    std::unique_ptr<TcpClient> client_;
};