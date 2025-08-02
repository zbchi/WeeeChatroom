#pragma once
#include <string>
#include <mutex>
#include "base.h"
class Neter;
class Client;
class FriendRequest;
class FriendService
{
public:
    FriendService(Neter *neter, Client *client) : neter_(neter),
                                                  client_(client) {}

    void getFriends();
    void handleFriendsList(const TcpConnectionPtr &conn, json &js);

    int addFriend(std::string &id);
    void handleAddFriendAck(const TcpConnectionPtr &conn, json &js);

    void delFriend(std::string &friend_id);
    int blockFriend(std::string &friend_id);
    void handleBlockFriendAck(const TcpConnectionPtr &conn, json &js);
    int unblockFriend(std::string &friend_id);
    void handleUnblockFriendAck(const TcpConnectionPtr &conn, json &js);

    void handleFriendRequest(const TcpConnectionPtr &conn, json &js);
    void responseFriendRequest(FriendRequest friendRequest, char *response);

    std::mutex friendRequests_mutex_;
    std::mutex friendList_mutex_;

    Waiter friendListWaiter_;

    Waiter friendAddWaiter_;

    Waiter blockWaiter_;
    Waiter unblockWaiter_;

private:
    Neter *neter_;
    Client *client_;
};

class Friend
{
public:
    std::string id_;
    std::string nickname_;
    bool isOnline_;
    bool is_blocked;
    void setCurrentFriend(Friend &friendObj)
    {
        id_ = friendObj.id_;
        nickname_ = friendObj.nickname_;
        isOnline_ = friendObj.isOnline_;
    }
    std::string user_id_;
};

class FriendRequest
{
public:
    std::string from_user_id;
    std::string nickname_;

    std::string timestamp_;
    std::string user_id_;
};
