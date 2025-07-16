#include "base.h"

void sendJson(const mylib::TcpConnectionPtr &conn, const json &js)
{
    std::string jsonStr = js.dump();
    int len = static_cast<int>(jsonStr.size());
    int beLen = htonl(len);
    std::string msg;
    msg.append(reinterpret_cast<const char *>(&beLen), sizeof(beLen));
    msg.append(jsonStr);
    conn->send(msg);
}

void sendJson(const mylib::TcpConnectionPtr &conn, const std::string &jsonStr)
{
    int len = static_cast<int>(jsonStr.size());
    int beLen = htonl(len);
    std::string msg;
    msg.append(reinterpret_cast<const char *>(&beLen), sizeof(beLen));
    msg.append(jsonStr);
    conn->send(msg);
}

json makeResponse(int msgid, int errno_, std::string errmsg)
{
    return {
        {"msgid", msgid},
        {"errmsg", errmsg},
        {"errno", errno_}};
}