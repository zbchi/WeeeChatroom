#include "base.h"

void sendJson(const mylib::TcpConnectionPtr &conn, json &js)
{
    std::string jsonStr = js.dump();
    int len = static_cast<int>(jsonStr.size());
    int beLen = htonl(len);
    std::string msg;
    msg.append(reinterpret_cast<const char *>(&beLen), sizeof(beLen));
    msg.append(jsonStr);
    conn->send(msg);
}