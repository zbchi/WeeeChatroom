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
    void start();

private:
    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;
    void handleMessage(const mylib::TcpConnectionPtr &conn, const std::string &jsonStr, mylib::Timestamp time);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    ThreadPool threadPool_;
    InetAddress listenAddr_;
    EventLoop loop_;
    TcpServer server_;
};