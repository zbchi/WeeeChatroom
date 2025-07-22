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

    void sendFriendRequestOffLine(std::string &to_user_id, const TcpConnectionPtr &conn);
    void sendMessageOffLine(std::string &to_user_id, const TcpConnectionPtr &conn);
    void sendGroupRequestOffLine(std::string &to_user_id, const TcpConnectionPtr &conn);
    Service *service_;
};

class AccountKiller : public Handler
{
public:
    AccountKiller(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};
