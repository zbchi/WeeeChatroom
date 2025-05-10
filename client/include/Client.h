#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

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
    void reg(const TcpConnectionPtr &conn);
};