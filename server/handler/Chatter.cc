#include "Chatter.h"

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
    auto targetConn = service_->getConnectionPtr(receiver_id);
    if (targetConn != nullptr)
    { // 在线直接发送
        sendJson(targetConn, js);
    }
    else
    { // 离线存储消息
        mysql->insert("offlineMessages", {{"sender_id", user_id},
                                          {"receiver_id", receiver_id},
                                          {"content", content},
                                          {"json", js.dump()}});
    }
    mysql->insert("messages", {{"sender_id", user_id},
                               {"receiver_id", receiver_id},
                               {"content", content}});
}
