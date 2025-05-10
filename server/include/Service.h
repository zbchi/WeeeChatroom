#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace mylib;
class Service
{
public:
    Service();
    enum MsgType
    {
        REG_MSG = 1,
        REG_MSG_ACK,
        LOGIN_MSG,
        LOGIN_MSG_ACK
    };
    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    
    void handleMessage(const TcpConnectionPtr&conn,std::string &jsonStr,Timestamp time);
};