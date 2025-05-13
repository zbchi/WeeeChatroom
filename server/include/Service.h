#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include "MSG_TYPE.h"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace mylib;
class Service
{
public:
    Service();
    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time);
};