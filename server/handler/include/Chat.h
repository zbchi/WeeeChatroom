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

class GroupChatter : public Handler
{
public:
    GroupChatter(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};