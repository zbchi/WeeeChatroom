#pragma once
#include <string>
#include "Handler.h"
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

    Waiter regWaiter_;
    Waiter loginWaiter_;
private:
    Neter *neter_;
    Client *client_;
};