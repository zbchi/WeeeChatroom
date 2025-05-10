#include "Client.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

Client::Client()
{
}
void Client::reg(const TcpConnectionPtr &conn)
{
    json regInfo;
    regInfo["emmail"] = "1662308219@qq.com";
    regInfo["password"] = "1234567890";

    std::string msg = regInfo.dump();
    conn->send(msg);
}