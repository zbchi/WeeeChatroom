#include "Handler.h"

using namespace mylib;
class Service;
class Chatter : public Handler
{
public:
    Chatter(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};