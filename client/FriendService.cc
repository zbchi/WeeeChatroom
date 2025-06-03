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

void FriendService::delFriend(std::string &friend_id)
{
    json delInfo;
    delInfo["msgid"] = DEL_FRIEND;
    delInfo["to_user_id"] = friend_id;
    delInfo["from_user_id"] = client_->user_id_;
    neter_->sendJson(delInfo);
}

void FriendService::handleFriendRequest(const TcpConnectionPtr &conn, json &js)
{
    std::string from_user_id = js["from_user_id"];
    std::string from_user_nickname = js["from_user_nickname"];
    std::string timestamp = js["timestamp"];

    // 将好友请求存入friendRequests_
    FriendRequest rq{from_user_id, from_user_nickname, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(friendRequests_mutex_);
        client_->friendRequests_.push_back(rq);
    }
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
    client_->friendList_.clear();
    Friend f;
    for (const auto &afriend : js["friends"])
    {
        f.id_ = afriend["id"];
        f.nickname_ = afriend["nickname"];
        f.user_id_ = client_->user_id_;
        client_->friendList_.push_back(f);
    }
}

void FriendService::responseFriendRequest(FriendRequest &friendRequest, char *response)
{
    json acceptInfo;
    acceptInfo["msgid"] = ADD_FRIEND_ACK;
    acceptInfo["from_user_id"] = friendRequest.from_user_id;
    acceptInfo["to_user_id"] = friendRequest.user_id_;
    acceptInfo["response"] = std::string(response);
    neter_->sendJson(acceptInfo);

    for (auto it = client_->friendRequests_.begin(); it != client_->friendRequests_.end();)
    {
        if (it->from_user_id == friendRequest.from_user_id &&
            it->user_id_ == friendRequest.user_id_)
        {
            it = client_->friendRequests_.erase(it);
        }
        else
            it++;
    }
}
