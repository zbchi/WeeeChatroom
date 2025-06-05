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
    if (state_ == State::SHOW_GROUPS)
        client_->controller_.flushGroups();
}