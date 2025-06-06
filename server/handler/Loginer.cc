#include "Loginer.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
#include <curl/curl.h>
void Loginer::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string email = js["email"].get<std::string>();
    std::string password = js["password"].get<std::string>();

    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["email"] = email;
    int errno_verify = verifyAccount(email, password, conn);
    if (errno_verify == 0)
    {
        response["errno"] = 0;
        response["user_id"] = service_->getUserid(conn);
        response["errmsg"] = "";
    }
    else if (errno_verify == 2)
    {
        response["errno"] = 2;
        response["errmsg"] = "该邮箱未注册";
    }
    else if (errno_verify == 1)
    {
        response["errno"] = 1;
        response["errmsg"] = "密码错误";
    }
    sendJson(conn, response);

    if (errno_verify == 0)
    {
        // 用户上线自动发送待发送的好友请求
        std::string user_id = service_->getUserid(conn);
        sendFriendRequestOffLine(user_id, conn);
        // 发送离线消息
        sendMessageOffLine(user_id, conn);
        // 发送离线管理员加群申请
        sendGroupRequestOffLine(user_id, conn);
        // 发送离线移除group add request
        // sendGroupRequestRemoveOffline(user_id,conn);
    }
}

int Loginer::verifyAccount(std::string &email, std::string &password, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();

    auto result = mysql->select("users", {{"email", email}});

    if (result.empty())
    {
        LOG_DEBUG("%s未注册", email.c_str());
        return 2;
    }

    if (result[0]["password"] == password)
    {
        std::lock_guard<std::mutex> lock(service_->onlienUsersMutex_);
        service_->onlienUsers_[result[0]["id"]] = conn;
        LOG_DEBUG("密码正确");
        return 0;
    }
    else
    {
        LOG_DEBUG("密码错误");
        return 1;
    }
}

void Loginer::sendFriendRequestOffLine(std::string &to_user_id, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("friend_requests", {{"to_user_id", to_user_id}});
    for (const auto &row : result)
    {
        json js = json::parse(row.at("json"));
        sendJson(conn, js);

        mysql->del("friend_requests", {{"id", row.at("id")}});
    }
}

void Loginer::sendMessageOffLine(std::string &to_user_id, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("offlineMessages", {{"receiver_id", to_user_id}});
    for (const auto &row : result)
    {
        json js = json::parse(row.at("json"));
        sendJson(conn, js);

        mysql->del("offlineMessages", {{"id", row.at("id")}});
    }
}

void Loginer::sendGroupRequestOffLine(std::string &to_user_id, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("group_requests", {{"to_user_id", to_user_id}});
    for (const auto &row : result)
    {
        json js = json::parse(row.at("json"));
        sendJson(conn, js);

        mysql->del("group_requests", {{"id", row.at("id")}});
    }
}