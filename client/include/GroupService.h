#pragma once
#include <string>
#include <mutex>
#include "Handler.h"
#include "FriendService.h"
class Neter;
class Client;
class GroupAddRequest;
class GroupUser : public Friend
{
public:
    std::string role_;
};

class Group
{
public:
    std::string group_id_;
    std::string group_name;
    void setCurrentGroup(Group &group)
    {
        group_id_ = group.group_id_;
        group_name = group.group_name;
    }
    std::string user_id_;
};

class GroupService
{
public:
    GroupService(Neter *neter, Client *client) : neter_(neter),
                                                 client_(client) {}
    void getGroups();
    void createGroup(std::string &groupname, std::string &description);
    void addGroup(std::string &group_id);
    void handleGroupRequest(const TcpConnectionPtr &conn, json &js);
    void handleGroupList(const TcpConnectionPtr &conn, json &js);
    void responseGroupRequest(GroupAddRequest &groupAddRequest, char *response);
    std::mutex groupAddRequests_mutex_;
    void handleGroupRequestRemove(const TcpConnectionPtr &conn, json &js);

private:
    void removeGroupAddRequest(std::string &group_id, std::string &from_user_id);
    Neter *neter_;
    Client *client_;
};

class GroupAddRequest
{
public:
    std::string from_user_id;
    std::string group_id;
    std::string nickname;
    std::string group_name;
    std::string timestamp;

    std::string user_id;
};
