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

    // auto result = mysql->select("friends", {{"user_id", user_id},
    //                                        {"friend_id", receiver_id}});
    // if (!result.empty()) // 判断是否为好友
    if (redis->sismember("friends:" + user_id, receiver_id))
    {
        auto targetConn = service_->getConnectionPtr(receiver_id);

        // auto result = mysql->select("friends", {{"user_id", receiver_id},
        //                                         {"friend_id", user_id}});

        // if (!result.empty()) // 再判断发送方是否为接收方的好友
        if (redis->sismember("friends:" + receiver_id, user_id))
        {
            if (targetConn != nullptr)
            { // 在线直接发送
                sendJson(targetConn, js);
            }
            else
            { // 离线存储离线消息
                LOG_DEBUG("存储离线消息");
                mysql->insert("offlineMessages", {{"sender_id", user_id},
                                                  {"receiver_id", receiver_id},
                                                  {"json", js.dump()}});
            }
            // 无论是否在线 存储消息
            /* mysql->insert("messages", {{"sender_id", user_id},
                                        {"receiver_id", receiver_id},
                                        {"content", content}});*/
        }
        else
            LOG_DEBUG("屏蔽好友关系的消息");
    }
    else
    {
        chat_errno = 1; // 非好友
        LOG_DEBUG("非好友关系的消息");
    } // sendJson(conn, makeResponse(CHAT_MSG_ACK, chat_errno));
    LOG_DEBUG("处理聊天消息完成");
}

void GroupChatter::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string sender_id = js["sender_id"];
    std::string group_id = js["group_id"];
    std::string content = js["content"];

    auto mysql = MySQLConnPool::instance().getConnection();
    int chat_errno = 0;
    // 判断是否在群里
    /*auto result = mysql->select("group_members", {{"group_id", group_id},
                                                  {"user_id", sender_id}});*/
    if (redis->sismember("group:" + group_id, sender_id))
    {
        // auto members = mysql->select("group_members", {{"group_id", group_id}});
        //  遍历群成员id
        std::unordered_set<std::string> members;
        redis->smembers("group:" + group_id, std::inserter(members, members.begin()));
        for (const auto &member_id : members)
        {
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
            /* mysql->insert("group_messages", {{"group_id", group_id},
                                              {"sender_id", sender_id},
                                              {"content", content}});*/
        }
    }
    else
    {
        chat_errno = 1; // 不在群里面
        LOG_DEBUG("非群成员的消息");
    }
    sendJson(conn, makeResponse(CHAT_GROUP_MSG_ACK, chat_errno));
}
