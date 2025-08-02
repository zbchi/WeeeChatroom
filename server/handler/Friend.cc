#include "Friend.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Chat.h"

#include "Redis.h"
#include <curl/curl.h>
#include <vector>
void FriendLister::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string user_id = js["user_id"];
    sendFriendList(user_id);
}

void FriendLister::sendFriendList(const std::string &user_id)
{
    auto conn = service_->getConnectionPtr(user_id);
    if (conn == nullptr)
        return;
    auto friendsId = getFriendsId(user_id);
    auto friends = getFriendsInfo(friendsId);

    json friendList;
    friendList["msgid"] = GET_FRIENDS;
    for (const auto &user : friends)
    {
        json u;
        if (redis->sismember("blacklist:" + user.at("id"), user_id)) // 被拉黑了
        {
            LOG_DEBUG("有拉黑痕迹");
            u["is_blocked"] = true;
        }
        else
            u["is_blocked"] = false;
        u["id"] = user.at("id");
        if (service_->getConnectionPtr(user.at("id")) == nullptr)
            u["isOnline"] = false;
        else
            u["isOnline"] = true;
        u["nickname"] = user.at("nickname");

        friendList["friends"].push_back(u);
    }
    sendJson(conn, friendList);
}

Result FriendLister::getFriendsId(const std::string &user_id)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    return mysql->select("friends", {{"status", "accepted"},
                                     {"user_id", user_id}});
}

Result FriendLister::getFriendsInfo(Result &friendsId)
{
    if (friendsId.empty())
        return {};
    auto mysql = MySQLConnPool::instance().getConnection();
    std::vector<std::string> id_list;
    for (const auto &friend_map : friendsId)
    {
        // std::cout << friend_map.at("friend_id");
        id_list.push_back(friend_map.at("friend_id"));
    }

    return mysql->select("users", {}, {{"id", id_list}});
}

void FriendAdder::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string to_user_id = js["id"];
    std::string from_user_id = js["from_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("users", {{"id", to_user_id}});
    if (result.empty())
    {
        sendJson(conn, makeResponse(ADD_FRIEND_ACK, 1)); // 1  不存在该用户
        return;
    }
    else if (to_user_id == from_user_id)
    {
        sendJson(conn, makeResponse(ADD_FRIEND_ACK, 2)); // 2  不能添加自己
        return;
    }
    else if (redis->sismember("friends:" + from_user_id, to_user_id))
    {
        sendJson(conn, makeResponse(ADD_FRIEND_ACK, 3)); // 3  你们已经是好友关系
        return;
    }

    result = mysql->select("friend_requests", {{"from_user_id", from_user_id},
                                               {"to_user_id", to_user_id}});
    if (!result.empty())
    {
        sendJson(conn, makeResponse(ADD_FRIEND_ACK, 4)); // 4 已经有一条一样的申请
        return;
    }

    js["from_user_nickname"] = mysql->getNicknameById(from_user_id);

    std::string jsonStr = js.dump();

    auto targetConn = service_->getConnectionPtr(to_user_id);
    if (targetConn != nullptr)
    { // 在线直接转发
        sendJson(targetConn, js);
    }

    // 无论是否在线存储申请记录
    mysql->insert("friend_requests", {{"to_user_id", to_user_id},
                                      {"from_user_id", from_user_id},
                                      {"json", js.dump()}});
    sendJson(conn, makeResponse(ADD_FRIEND_ACK, 0));
    LOG_INFO("[%s]请求添加[%s]", from_user_id, to_user_id);
}

void FriendAddResponser::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 处理好友请求的回应
    std::string response = js["response"];
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    if (response == "accept")
    {
        // 更新缓存
        redis->sadd("friends:" + from_user_id, to_user_id);
        redis->sadd("friends:" + to_user_id, from_user_id);
        mysql->insert("friends", {{"user_id", from_user_id},
                                  {"friend_id", to_user_id}});

        mysql->insert("friends", {{"user_id", to_user_id},
                                  {"friend_id", from_user_id}});

        // 清除拉黑痕迹
        redis->srem("blacklist:" + from_user_id, to_user_id);
        redis->srem("blacklist:" + to_user_id, from_user_id);
    }
    else if (response == "reject")
    { // 通知被拒绝
        std::string email = mysql->getEmailById(to_user_id);
        json jss;
        jss["msgid"] = CHAT_MSG;
        jss["sender_id"] = "1";
        jss["receiver_id"] = from_user_id;
        jss["content"] = "你向好友[" + email + "]发送的好友请求被拒绝了";
        jss["nickname"] = "系统消息";
        std::string timestamp = Timestamp::now().toFormattedString();
        jss["timestamp"] = timestamp;
        Chatter chatter(service_);
        chatter.handle(conn, jss, time);
    }
    // 处理完后删除申请记录
    mysql->del("friend_requests", {{"to_user_id", to_user_id},
                                   {"from_user_id", from_user_id}});

    // 更新用户好友列表
    FriendLister list(service_);
    list.sendFriendList(from_user_id);
    list.sendFriendList(to_user_id);
    LOG_INFO("[%s]处理来自[%s]的好友请求", to_user_id.c_str(), from_user_id.c_str());
}

void FriendDeleter::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];

    auto mysql = MySQLConnPool::instance().getConnection();

    // 更新缓存
    redis->srem("friends:" + from_user_id, to_user_id);
    redis->srem("friends:" + to_user_id, from_user_id);
    mysql->del("friends", {{"user_id", from_user_id},
                           {"friend_id", to_user_id}});
    mysql->del("friends", {{"user_id", to_user_id},
                           {"friend_id", from_user_id}});

    // 更新用户好友列表
    FriendLister list(service_);
    list.sendFriendList(from_user_id);
    list.sendFriendList(to_user_id);
    LOG_INFO("[%s]删除好友[%s]", from_user_id.c_str(), to_user_id.c_str());
}

void FriendBlocker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];

    if (redis->sismember("blacklist:" + from_user_id, to_user_id))
    {
        sendJson(conn, makeResponse(BLOCK_FRIEND_ACK, 1));
        return; // 已经将此好友拉黑
    }
    // 将黑名单存入缓存
    redis->sadd("blacklist:" + from_user_id, to_user_id);

    sendJson(conn, makeResponse(BLOCK_FRIEND_ACK, 0));
    // 更新被拉黑用户的好友列表
    FriendLister list(service_);
    list.sendFriendList(to_user_id);

    // auto mysql = MySQLConnPool::instance().getConnection();
    //  删掉单向好友关系，保留对端的好友关系
    /*mysql->del("friends", {{"user_id", from_user_id},
                           {"friend_id", to_user_id}});*/
    LOG_INFO("[%s]将好友[%s]拉黑", from_user_id.c_str(), to_user_id.c_str());
}

void FriendUnblocker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];
    if (!redis->sismember("blacklist:" + from_user_id, to_user_id))
    {
        sendJson(conn, makeResponse(UNBLOCK_FRIEND_ACK, 1));
        return; // 没有将此好友拉黑
    }
    redis->srem("blacklist:" + from_user_id, to_user_id);
    sendJson(conn, makeResponse(UNBLOCK_FRIEND_ACK, 0));

    // 更新被解除拉黑用户的好友列表
    FriendLister list(service_);
    list.sendFriendList(to_user_id);
    LOG_INFO("[%s]将好友[%s]解除拉黑", from_user_id.c_str(), to_user_id.c_str());
}