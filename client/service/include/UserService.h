#pragma once
#include <string>
#include "base.h"
class Neter;
class Client;
class UserService
{
public:
    UserService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}

    int registerCode(std::string &email, std::string &password, std::string &nickname, int code);

    void regiSter(std::string &email, std::string &password, std::string &nickname);
    int login(std::string &email, std::string &password);
    void handleLoginAck(const TcpConnectionPtr &conn, json &js);
    void handleRegAck(const TcpConnectionPtr &conn, json &js);
    void handleFindAck(const TcpConnectionPtr &conn, json &js);

    void findPassword(std::string &email);
    int findPasswordCode(std::string &email, std::string &password, int code);
    Waiter regWaiter_;
    Waiter loginWaiter_;
    Waiter findWaiter_;

private:
    Neter *neter_;
    Client *client_;
};