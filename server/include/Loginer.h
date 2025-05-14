#include "Handler.h"
using namespace mylib;
class Service;
class Loginer : public Handler
{
public:
    Loginer(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};