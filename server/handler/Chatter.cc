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
    std::string reciever_id = js["reciver_id"];
    std::string content = js["content"];
    auto targetConn = service_->getConnectionPtr(user_id);
    if (targetConn != nullptr)
    {
        sendJson(targetConn, js);
        char sql[128];
        snprintf(sql, sizeof(sql), "insert into messages(sender_id,receiver_id,content) values('%s','%s','%s')",
                 user_id.c_str(), reciever_id.c_str(), content.c_str());
        mysql->update(std::string(sql));
    }
    else
    {
        char sql[128];
        snprintf(sql, sizeof(sql), "insert into offlineMessages(sender_id,receiver_id,content) values('%s','%s','%s')",
                 user_id.c_str(), reciever_id.c_str(), content.c_str());
        mysql->update(std::string(sql));
    }
}
