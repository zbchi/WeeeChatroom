#pragma once
#include <string>
#include "Handler.h"
class Neter;
class Client;
class FriendService
{
public:
    FriendService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}

    void addFriend(std::string &friend_id);
    void handleFriendRequest(const TcpConnectionPtr &conn, json &js);
    void getFriends();
    void handleFriendsList(const TcpConnectionPtr &conn, json &js);

private:
    Neter *neter_;
    Client *client_;
};