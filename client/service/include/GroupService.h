#pragma once
#include "FriendService.h"
class Neter;
class Client;
class GroupAddRequest;
class GroupMember : public Friend
{
public:
    std::string role_;
};

class Group
{
public:
    std::string group_id_;
    std::string group_name;
    std::string user_id_;
    std::map<std::string, GroupMember> group_members;

    void setCurrentGroup(Group &group)
    {
        group_id_ = group.group_id_;
        group_name = group.group_name;
    }
};

class GroupService
{
public:
    GroupService(Neter *neter, Client *client) : neter_(neter),
                                                 client_(client) {}
    void getGroups();
    void getGroupInfo();
    void createGroup(std::string &groupname, std::string &description);
    void addGroup(std::string &group_id);
    void handleGroupRequest(const TcpConnectionPtr &conn, json &js);
    void handleGroupList(const TcpConnectionPtr &conn, json &js);
    void responseGroupRequest(GroupAddRequest &groupAddRequest, char *response);
    std::mutex groupAddRequests_mutex_;
    void handleGroupRequestRemove(const TcpConnectionPtr &conn, json &js);
    void handleGroupInfo(const TcpConnectionPtr &conn, json &js);
    void exitGroup();
    void kickMember(std::string &user_id);
    void addAdmin(std::string &to_user_id);
    void removeAdmin(std::string &to_user_id);

    Waiter groupListWaiter_;
    Waiter groupInfoWaiter_;

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
