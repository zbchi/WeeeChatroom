#include "Friend.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include <curl/curl.h>
#include <vector>
void FriendLister::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string user_id = js["user_id"];
    sendFriendList(user_id);
}

void FriendLister::sendFriendList(std::string &user_id)
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

Result FriendLister::getFriendsId(std::string &user_id)
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
    std::string email = js["email"];
    std::string from_user_id = js["from_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    std::string to_user_id = mysql->getIdByEmail(email);
    if (to_user_id == "")
        return;
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
}

void FriendAddAcker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
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
    }
    else if (response == "reject")
    {
    }
    // 处理完后删除申请记录
    mysql->del("friend_requests", {{"to_user_id", to_user_id},
                                   {"from_user_id", from_user_id}});

    // 更新用户好友列表
    FriendLister list(service_);
    list.sendFriendList(from_user_id);
    list.sendFriendList(to_user_id);
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
}

void FriendBlocker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];

    auto mysql = MySQLConnPool::instance().getConnection();
    // 删掉单向好友关系，保留对端的好友关系
    mysql->del("friends", {{"user_id", from_user_id},
                           {"friend_id", to_user_id}});
}