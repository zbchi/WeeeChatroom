#include "Friender.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
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
        u["nickname"] = user.at("nickname");
        friendList["friends"].push_back(u);
    }

    sendJson(conn, friendList);
}

std::vector<std::map<std::string, std::string>> FriendLister::getFriendsId(std::string user_id_str)
{

    auto mysql = MySQLConnPool::instance().getConnection();
    return mysql->select("friends", {{"status", "accepted"},
                                     {"user_id", user_id_str}});
}

std::vector<std::map<std::string, std::string>> FriendLister::getFriendsInfo(std::vector<std::map<std::string, std::string>> &friendsId)
{
    auto mysql = MySQLConnPool::instance().getConnection();

    std::vector<std::string> id_list;
    for (const auto &friend_map : friendsId)
    {
        std::cout << friend_map.at("friend_id");
        id_list.push_back(friend_map.at("friend_id"));
    }

    return mysql->select("users", {}, {{"id", id_list}});
}

void FriendAdder::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string to_user_id = js["to_user_id"];
    std::string from_user_id = js["from_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    js["from_user_nickname"] = mysql->getNicknameById(from_user_id);

    std::string jsonStr = js.dump();
    std::cout << jsonStr << std::endl;

    auto targetConn = service_->getConnectionPtr(to_user_id);
    if (targetConn != nullptr)
    { // 在线直接转发
        sendJson(targetConn, js);
    }
    else
    { // 离线存储申请记录
        mysql->insert("friend_requests", {{"to_user_id", to_user_id},
                                          {"from_user_id", from_user_id},
                                          {"json", js.dump()}});
    }
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
        mysql->insert("friends", {{"user_id", from_user_id},
                                  {"friend_id", to_user_id}});

        mysql->insert("friends", {{"user_id", to_user_id},
                                  {"friend_id", from_user_id}});
    }
    else if (response == "reject")
    {
        std::cout << "rejectrejectrejectrejectreject" << std::endl;
    }

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
    mysql->del("friends", {{"user_id", from_user_id},
                           {"friend_id", to_user_id}});
    mysql->del("friends", {{"user_id", to_user_id},
                           {"friend_id", from_user_id}});

    // 更新用户好友列表
    FriendLister list(service_);
    list.sendFriendList(from_user_id);
    list.sendFriendList(to_user_id);
}
