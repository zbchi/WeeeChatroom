#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>

#include "InetAddress.h"
#include "TcpClient.h"
#include "EventLoop.h"
using namespace mylib;
class Client
{
public:
    Client();
    enum MsgType
    {
        REG_MSG = 1,
        REG_MSG_ACK,
        LOGIN_MSG,
        LOGIN_MSG_ACK
    };
    using MsgHandler = std::function<void(const TcpConnection &, json &, Timetamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time);
    void reg(const TcpConnectionPtr &conn);
    void parse();
};