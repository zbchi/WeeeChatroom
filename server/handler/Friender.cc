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
        char sql[256];
        snprintf(sql, sizeof(sql),
                 "insert into friend_requests(to_user_id,from_user_id,json) values('%s','%s','%s')",
                 to_user_id.c_str(), from_user_id.c_str(), js.dump().c_str());
        mysql->update(std::string(sql));
    }
}

void FriendAddAcker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string response = js["response"];
    std::string from_user_id = js["from_user_id"];
    std::string to_user_id = js["to_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    if (response == "accept")
    {
        char sql[256];
        snprintf(sql, sizeof(sql),
                 "insert into friends(user_id,friend_id) values('%s','%s')",
                 from_user_id.c_str(), to_user_id.c_str());
        mysql->update(std::string(sql));
        snprintf(sql, sizeof(sql),
                 "insert into friends(user_id,friend_id) values('%s','%s')",
                 to_user_id.c_str(), from_user_id.c_str());
        mysql->update(std::string(sql));
    }
    else if (response == "reject")
    {
        std::cout << "rejectrejectrejectrejectreject" << std::endl;
    }
}
