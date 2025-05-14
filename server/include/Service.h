#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"
#include "base.h"

#include "Handler.h"
#include "ThreadPool.h"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class Service
{
public:
    Service();
    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;

    void handleMessage(const mylib::TcpConnectionPtr &conn, const std::string &jsonStr, mylib::Timestamp time);
    ThreadPool threadPool_;

private:
};