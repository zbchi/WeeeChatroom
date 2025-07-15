#include "Chat.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"

void Chatter::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    std::string user_id = js["sender_id"];
    std::string receiver_id = js["receiver_id"];
    std::string content = js["content"];

    int chat_errno = 0;

    auto result = mysql->select("friends", {{"user_id", user_id},
                                            {"friend_id", receiver_id}});
    if (!result.empty()) // 判断是否为好友
    {
        auto targetConn = service_->getConnectionPtr(receiver_id);

        auto result = mysql->select("friends", {{"user_id", receiver_id},
                                                {"friend_id", user_id}});

        if (!result.empty()) // 再判断是否屏蔽
        {
            if (targetConn != nullptr)
                // 在线直接发送
                sendJson(targetConn, js);

            else
            { // 离线存储离线消息
                mysql->insert("offlineMessages", {{"sender_id", user_id},
                                                  {"receiver_id", receiver_id},
                                                  {"json", js.dump()}});
            }
            // 无论是否在线 存储消息
            mysql->insert("messages", {{"sender_id", user_id},
                                       {"receiver_id", receiver_id},
                                       {"content", content}});
        }
    }
    else
        chat_errno = 1; // 非好友
    sendJson(conn, makeResponse(CHAT_MSG_ACK, chat_errno));
}

void GroupChatter::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string sender_id = js["sender_id"];
    std::string group_id = js["group_id"];
    std::string content = js["content"];

    auto mysql = MySQLConnPool::instance().getConnection();
    int chat_errno = 0;
    // 判断是否在群里
    auto result = mysql->select("group_members", {{"group_id", group_id},
                                                  {"user_id", sender_id}});
    if (!result.empty())
    {
        auto members = mysql->select("group_members", {{"group_id", group_id}});
        // 遍历群成员id
        for (const auto &member : members)
        {
            std::string member_id = member.at("user_id");
            if (member_id != sender_id)
            {
                auto targetConn = service_->getConnectionPtr(member_id);
                if (targetConn != nullptr) // 在线直接转发
                {
                    sendJson(targetConn, js);
                }
                else // 离线存储离线消息
                {
                    mysql->insert("offlineMessages", {{"sender_id", sender_id},
                                                      {"receiver_id", member_id},
                                                      {"json", js.dump()}});
                }
            }
            mysql->insert("group_messages", {{"group_id", group_id},
                                             {"sender_id", sender_id},
                                             {"content", content}});
        }
    }
    else
        chat_errno = 1; // 不在群里面
    sendJson(conn, makeResponse(CHAT_GROUP_MSG_ACK, chat_errno));
}
