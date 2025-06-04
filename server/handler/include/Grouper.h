#pragma once
#include "Handler.h"
using namespace mylib;
class Service;
class GroupCreater : public Handler
{
public:
    GroupCreater(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};