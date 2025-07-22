#include "Login.h"
#include "Friend.h"
#include "Group.h"

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
        LOG_WARN("[%s]已登录", email.c_str());
        return 3;
    }
    if (result.empty())
    {
        LOG_WARN("[%s]未注册", email.c_str());
        return 2;
    }

    if (result[0]["password"] == password)
    {
        std::lock_guard<std::mutex> lock(service_->onlienUsersMutex_);
        service_->onlineUsers_[result[0]["id"]] = conn;
        LOG_INFO("[%s]密码正确", email.c_str());
        return 0;
    }
    else
    {
        LOG_WARN("[%s]密码错误", email.c_str());
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
    // auto mysql = MySQLConnPool::instance().getConnection();
    // auto result = mysql->select("offlineMessages", {{"receiver_id", to_user_id}});

    std::string redis_key = "offlineMessages:" + to_user_id;
    while (1)
    {
        auto targetConn = service_->getConnectionPtr(to_user_id);
        if (targetConn == nullptr)
            return; // 断线后停止发送
        std::optional<std::string> json = redis->rpop(redis_key);
        if (!json.has_value())
            break;
        sendJson(conn, json.value());
        LOG_INFO("发送离线消息给[%s]", to_user_id.c_str());
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

    // 清除在线状态
    {
        std::lock_guard<std::mutex> lock(service_->onlienUsersMutex_);
        for (auto it = service_->onlineUsers_.begin(); it != service_->onlineUsers_.end(); it++)
        {
            if (it->second == conn)
            {
                service_->onlineUsers_.erase(it);
                break;
            }
        }
    }
    auto mysql = MySQLConnPool::instance().getConnection();

    // 解散是群主的群
    auto groups = mysql->select("`groups`", {{"creator_id", user_id}});
    for (const auto &group : groups)
    {
        json js;
        js["group_id"] = group.at("id");
        js["user_id"] = user_id;
        GroupExiter groupExiter(service_);
        groupExiter.handle(conn, js, time);
    }

    // 清除离线消息
    mysql->del("offlineMessages", {{"sender_id", user_id}});
    mysql->del("offlineMessages", {{"receiver_id", user_id}});

    // 清除拉黑关系
    redis->del("blacklist:" + user_id);

    // 清除加好友请求和加群请求
    mysql->del("friend_requests", {{"from_user_id", user_id}});
    mysql->del("friend_requests", {{"to_user_id", user_id}});

    mysql->del("group_requests", {{"from_user_id", user_id}});
    mysql->del("group_requests", {{"to_user_id", user_id}});

    // 清除好友关系
    mysql->del("friends", {{"user_id", user_id}});
    mysql->del("friends", {{"friend_id", user_id}});
    redis->del("friends:" + user_id);
    auto users = mysql->select("users");
    for (const auto &user : users)
    {
        if (redis->srem("friends:" + user.at("id"), user_id))
        {
            // 更新销毁用户的好友的好友列表
            FriendLister list(service_);
            list.sendFriendList(user.at("id"));
        }
    }

    // 清除群关系
    mysql->del("group_members", {{"user_id", user_id}});
    auto groupss = mysql->select("`groups`");
    for (const auto &group : groupss)
        redis->srem("group:" + group.at("id"), user_id);

    // 删除用户表中的用户
    mysql->del("users", {{"id", user_id}});
    LOG_INFO("[%s]要求注销账户", user_id.c_str());
}
