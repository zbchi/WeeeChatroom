#include "FriendService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"

void FriendService::addFriend(std::string &friend_id)
{
    json addInfo;
    addInfo["msgid"] = ADD_FRIEND;
    addInfo["to_user_id"] = friend_id;
    addInfo["from_user_id"] = client_->user_id_;
    std::string timestamp = Timestamp::now().toFormattedString();
    addInfo["timestamp"] = timestamp;
    neter_->sendJson(addInfo);
}

void FriendService::handleFriendRequest(const TcpConnectionPtr &conn, json &js)
{
    
}

void FriendService::getFriends()
{
    json getInfo;
    getInfo["msgid"] = GET_FRIENDS;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);
}

void FriendService::handleFriendsList(const TcpConnectionPtr &conn, json &js)
{

    Friend f;
    for (const auto &afriend : js["friends"])
    {
        f.id_ = afriend["id"];
        f.nickname_ = afriend["nickname"];
        client_->firendList_.push_back(f);
    }
}