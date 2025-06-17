#include "GroupService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

void GroupService::getGroups()
{
    json getInfo;
    getInfo["msgid"] = GET_GROUPS;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);

    groupListWaiter_.wait();
}

void GroupService::createGroup(std::string &groupname, std::string &description)
{
    json groupInfo;
    groupInfo["msgid"] = CREATE_GROUP;
    groupInfo["creator_id"] = client_->user_id_;
    groupInfo["name"] = groupname;
    groupInfo["description"] = description;
    neter_->sendJson(groupInfo);
}

void GroupService::addGroup(std::string &group_id)
{
    json addInfo;
    addInfo["msgid"] = ADD_GROUP;
    addInfo["group_id"] = group_id;
    addInfo["from_user_id"] = client_->user_id_;
    std::string timestamp = Timestamp::now().toFormattedString();
    addInfo["timestamp"] = timestamp;
    neter_->sendJson(addInfo);
}

void GroupService::handleGroupRequest(const TcpConnectionPtr &conn, json &js)
{
    std::string group_id = js["group_id"];
    std::string group_name = js["group_name"];
    std::string nickname = js["nickname"];
    std::string user_id = js["from_user_id"];
    std::string timestamp = js["timestamp"];
    // 将入群请求存入groupRequests_
    GroupAddRequest rq{user_id, group_id, nickname, group_name, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(groupAddRequests_mutex_);
        client_->groupAddRequests_.push_back(rq);
    }
}
void GroupService::responseGroupRequest(GroupAddRequest &groupAddRequest, char *response)
{
    json acceptInfo;
    acceptInfo["msgid"] = ADD_GROUP_ACK;
    acceptInfo["group_id"] = groupAddRequest.group_id;
    acceptInfo["from_user_id"] = groupAddRequest.from_user_id;
    acceptInfo["response"] = std::string(response);
    neter_->sendJson(acceptInfo);
    removeGroupAddRequest(groupAddRequest.group_id, groupAddRequest.from_user_id);
}

void GroupService::handleGroupList(const TcpConnectionPtr &conn, json &js)
{
    client_->groupList_.clear();
    Group g;
    for (const auto &agroup : js["groups"])
    {
        g.group_id_ = agroup["id"];
        g.group_name = agroup["name"];
        client_->groupList_.push_back(g);
    }
    groupListWaiter_.notify(1);
}

void GroupService::removeGroupAddRequest(std::string &group_id, std::string &from_user_id)
{
    for (auto it = client_->groupAddRequests_.begin(); it != client_->groupAddRequests_.end();)
    {
        if (it->group_id == group_id &&
            it->from_user_id == from_user_id)
        {
            it = client_->groupAddRequests_.erase(it);
        }
        else
            it++;
    }
}

void GroupService::handleGroupRequestRemove(const TcpConnectionPtr &conn, json &js)
{
    std::string from_user_id = js["from_user_id"];
    std::string group_id = js["group_id"];
    std::lock_guard<std::mutex> lock(groupAddRequests_mutex_);
    removeGroupAddRequest(group_id, from_user_id);
}

void GroupService::getGroupInfo()
{
    json getInfo;
    getInfo["msgid"] = GET_GROUPINFO;
    getInfo["group_id"] = client_->currentGroup_.group_id_;
    neter_->sendJson(getInfo);
    groupInfoWaiter_.wait();
}

void GroupService::handleGroupInfo(const TcpConnectionPtr &conn, json &js)
{
    client_->currentGroup_.group_members.clear();
    GroupMember m;
    for (const auto &amember : js["members"])
    {
        m.id_ = amember["user_id"];
        m.nickname_ = amember["nickname"];
        m.role_ = amember["role"];
        client_->currentGroup_.group_members[m.id_] = m;
    }
    groupInfoWaiter_.notify(0);
}

void GroupService::exitGroup()
{
    json exitInfo;
    exitInfo["msgid"] = EXIT_GROUP;
    exitInfo["user_id"] = client_->user_id_;
    exitInfo["group_id"] = client_->currentGroup_.group_id_;
    neter_->sendJson(exitInfo);
}

void GroupService::kickMember(std::string &user_id)
{
    json kickInfo;
    kickInfo["msgid"] = KICK_MEMBER;
    kickInfo["kick_user_id"] = user_id;
    kickInfo["user_id"] = client_->user_id_;
    kickInfo["group_id"] = client_->currentGroup_.group_id_;
    neter_->sendJson(kickInfo);
}

void GroupService::addAdmin(std::string &to_user_id)
{
    json addInfo;
    addInfo["msgid"] = ADD_ADMIN;
    addInfo["to_user_id"] = to_user_id;
    addInfo["group_id"] = client_->currentGroup_.group_id_;
    addInfo["user_id"] = client_->user_id_;
    neter_->sendJson(addInfo);
}

void GroupService::removeAdmin(std::string &to_user_id)
{
    json rmInfo;
    rmInfo["msgid"] = REMOVE_ADMIN;
    rmInfo["to_user_id"] = to_user_id;
    rmInfo["group_id"] = client_->currentGroup_.group_id_;
    rmInfo["user_id"] = client_->user_id_;
    neter_->sendJson(rmInfo);
}