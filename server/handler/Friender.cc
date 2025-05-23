#include "Friender.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
#include <curl/curl.h>
void FriendLister::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string user_id = js["user_id"];
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
    int user_id = std::stoi(user_id_str);
    auto mysql = MySQLConnPool::instance().getConnection();
    char sql[128];
    snprintf(sql, sizeof(sql), "select friend_id from friends where status='accepted' and user_id=%d", user_id);
    return mysql->queryResult(std::string(sql));
}

std::vector<std::map<std::string, std::string>> FriendLister::getFriendsInfo(std::vector<std::map<std::string, std::string>> &friendsId)
{
    auto mysql = MySQLConnPool::instance().getConnection();

    std::stringstream id_list;
    for (size_t i = 0; i < friendsId.size(); i++)
    {
        if (i > 0)
            id_list << ",";
        id_list << friendsId[i]["friend_id"];
    }
    std::string sql = "select * from users where id in (" + id_list.str() + ")";
    return mysql->queryResult(sql);
}