#pragma once
#include <string>
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
    void addFriend(std::string &friendEmail);

    void getFriends();

private:
    Neter *neter_;
    Client *client_;
};