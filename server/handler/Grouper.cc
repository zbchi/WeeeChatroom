#include "Grouper.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "MySQLConn.h"

void GroupCreater::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string creator_id = js["creator_id"];
    std::string name = js["name"];
    std::string description = js["description"];
    auto mysql = MySQLConnPool::instance().getConnection();
    mysql->insert("`groups`", {{"creator_id", creator_id},
                               {"name", name},
                               {"description", description}});

    std::string group_id = std::to_string(mysql_insert_id(mysql->getConnection()));
    mysql->insert("group_members", {{"group_id", group_id},
                                    {"role", "owner"},
                                    {"user_id", creator_id}});
}