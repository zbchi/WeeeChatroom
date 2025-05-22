#pragma once
#include "Handler.h"
using namespace mylib;
class Service;
class Loginer : public Handler
{
public:
    Loginer(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    int verifyAccount(std::string &email, std::string &password, const TcpConnectionPtr &conn);
    Service *service_;
};