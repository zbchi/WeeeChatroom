#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace mylib;
void sendJson(const TcpConnectionPtr &conn, json &js)
{
    std::string jsonStr = js.dump();
    int len = static_cast<int>(jsonStr.size());
    int beLen = htonl(len);
    std::string msg;
    msg.append(reinterpret_cast<const char *>(&beLen), sizeof(beLen));
    msg.append(jsonStr);
    conn->send(msg);
}