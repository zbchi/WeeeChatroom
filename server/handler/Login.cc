#include "Login.h"

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
        std::string user_id = service_->getUserid(conn);
        response["user_id"] = user_id;
        auto mysql = MySQLConnPool::instance().getConnection();
        response["nickname"] = mysql->getNicknameById(user_id);
        response["errmsg"] = "";
    }
    else
        response["errno"] = errno_verify;

    sendJson(conn, response); // 1密码错误  2该邮箱未注册  3已经登录

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

    auto result = mysql->select("users", {{"email", email}, {"state", "alive"}});

    std::string user_id = mysql->getIdByEmail(email);
    auto conned = service_->getConnectionPtr(user_id);
    if (conned != nullptr)
    {
        LOG_DEBUG("%s已登录", email.c_str());
        return 3;
    }
    if (result.empty())
    {
        LOG_DEBUG("%s未注册", email.c_str());
        return 2;
    }

    if (result[0]["password"] == password)
    {
        std::lock_guard<std::mutex> lock(service_->onlienUsersMutex_);
        service_->onlineUsers_[result[0]["id"]] = conn;
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
        auto targetConn = service_->getConnectionPtr(to_user_id);
        if (targetConn == nullptr)
            return; // 断线后停止发送
        sendJson(conn, row.at("json"));
        // mysql->del("friend_requests", {{"id", row.at("id")}});  不删除，处理请求后再删除
    }
}

void Loginer::sendMessageOffLine(std::string &to_user_id, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("offlineMessages", {{"receiver_id", to_user_id}});
    for (const auto &row : result)
    {
        auto targetConn = service_->getConnectionPtr(to_user_id);
        if (targetConn == nullptr)
            return; // 断线后停止发送
        sendJson(conn, row.at("json"));
        LOG_DEBUG("发送离线消息");
        mysql->del("offlineMessages", {{"id", row.at("id")}});
    }
}

void Loginer::sendGroupRequestOffLine(std::string &to_user_id, const TcpConnectionPtr &conn)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("group_requests", {{"to_user_id", to_user_id}});
    for (const auto &row : result)
    {
        auto targetConn = service_->getConnectionPtr(to_user_id);
        if (targetConn == nullptr)
            return; // 断线后停止发送
        sendJson(conn, row.at("json"));
        // mysql->del("group_requests", {{"id", row.at("id")}});  不删除，处理请求后再删除
    }
}

void AccountKiller::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string user_id = js["user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    mysql->update("users", {{"state", "die"}}, {{"id", user_id}});
}
