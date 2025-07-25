#include "FriendService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"

#include "ui.h"
int FriendService::addFriend(std::string &email)
{
    json addInfo;
    addInfo["msgid"] = ADD_FRIEND;
    addInfo["email"] = email;
    addInfo["from_user_id"] = client_->user_id_;
    std::string timestamp = Timestamp::now().toFormattedString();
    addInfo["timestamp"] = timestamp;
    neter_->sendJson(addInfo);
    // 阻塞等待回应
    friendAddWaiter_.wait();
    return friendAddWaiter_.getResult();
}

void FriendService::handleAddFriendAck(const TcpConnectionPtr &conn, json &js)
{
    int add_errno = js["errno"];
    friendAddWaiter_.notify(add_errno);
}

void FriendService::delFriend(std::string &friend_id)
{
    json delInfo;
    delInfo["msgid"] = DEL_FRIEND;
    delInfo["to_user_id"] = friend_id;
    delInfo["from_user_id"] = client_->user_id_;
    neter_->sendJson(delInfo);
}

int FriendService::blockFriend(std::string &friend_id)
{
    json blockInfo;
    blockInfo["msgid"] = BLOCK_FRIEND;
    blockInfo["to_user_id"] = friend_id;
    blockInfo["from_user_id"] = client_->user_id_;
    neter_->sendJson(blockInfo);
    blockWaiter_.wait();
    return blockWaiter_.getResult();
}

void FriendService::handleBlockFriendAck(const TcpConnectionPtr &conn, json &js)
{
    int block_errno = js["errno"];
    blockWaiter_.notify(block_errno);
}

int FriendService::unblockFriend(std::string &friend_id)
{
    json unblockInfo;
    unblockInfo["msgid"] = UNBLOCK_FRIEND;
    unblockInfo["from_user_id"] = client_->user_id_;
    unblockInfo["to_user_id"] = friend_id;
    neter_->sendJson(unblockInfo);
    unblockWaiter_.wait();
    return unblockWaiter_.getResult();
}

void FriendService::handleUnblockFriendAck(const TcpConnectionPtr &conn, json &js)
{
    int unblock_errno = js["errno"];
    unblockWaiter_.notify(unblock_errno);
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
    printTopBegin();
    std::cout << "收到了一条好友请求" << std::endl;
    printTopEnd();
}

void FriendService::getFriends()
{
    json getInfo;
    getInfo["msgid"] = GET_FRIENDS;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);
    friendListWaiter_.wait();
    friendListWaiter_.getResult();
}

void FriendService::handleFriendsList(const TcpConnectionPtr &conn, json &js)
{
    client_->friendList_.clear();
    Friend f;
    for (const auto &afriend : js["friends"])
    {
        f.id_ = afriend["id"];
        f.nickname_ = afriend["nickname"];
        f.isOnline_ = afriend["isOnline"];
        f.is_blocked = afriend["is_blocked"];
        f.user_id_ = client_->user_id_;
        client_->friendList_[f.id_] = f;
    }
    friendListWaiter_.notify(0);
}

void FriendService::responseFriendRequest(FriendRequest friendRequest, char *response)
{
    json acceptInfo;
    acceptInfo["msgid"] = FRIEND_REQUEST;
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
