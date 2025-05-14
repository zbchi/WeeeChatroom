#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>

#include "InetAddress.h"
#include "TcpClient.h"
#include "EventLoop.h"

using namespace mylib;
using json = nlohmann::json;
class Client
{
public:
    Client();
    
    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;
    void send(std::string &str);
    void recieve();

    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time);
    void reg(const TcpConnectionPtr &conn);
    void reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void parse();

private:
    int socketfd_;
};