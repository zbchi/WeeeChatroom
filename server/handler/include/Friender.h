#pragma once
#include "Handler.h"
using namespace mylib;
class Service;
class FriendLister : public Handler
{
public:
    FriendLister(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    std::vector<std::map<std::string, std::string>> getFriendsId(std::string user_id);
    std::vector<std::map<std::string, std::string>> getFriendsInfo(std::vector<std::map<std::string, std::string>> &friendsId);
    Service *service_;
};