#pragma once
#include <string>
#include "Handler.h"
class Neter;
class Client;
class UserService
{
public:
    UserService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}

    void registerCode(std::string &email, std::string &password, std::string &nickname, int code);

    void regiSter(std::string &email, std::string &password, std::string &nickname);
    void login(std::string &email, std::string &password);
    void handleLoginAck(const TcpConnectionPtr &conn, json &js);
    void handleRegAck(const TcpConnectionPtr &conn, json &js);
    void addFriend(std::string &friendEmail);

    void getFriends();
    void handleFriendsList(const TcpConnectionPtr &conn, json &js);

private:
    Neter *neter_;
    Client *client_;
};