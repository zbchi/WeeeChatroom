#include "Grouper.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

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

void GroupAdder::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string group_id = js["group_id"];
    std::string from_user_id = js["from_user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    auto admins = mysql->select("group_members", {{"group_id", group_id},
                                                  {"role", "admin"}});

    auto name = mysql->select("`groups`", {{"id", group_id}});
    js["group_name"] = name[0].at("name");
    auto user_name = mysql->select("users", {{"id", from_user_id}});
    js["nickname"] = user_name[0].at("nickname");
    // 遍历管理员id
    for (const auto &admin : admins)
    {
        std::string admin_id = admin.at("user_id");
        auto targetConn = service_->getConnectionPtr(admin_id);
        if (targetConn != nullptr) // 在线直接转发
        {
            sendJson(targetConn, js);
        }
        else // 离线存储加群请求记录，待管理员上线后发
        {
            js["to_user_id"] = admin_id;
            mysql->insert("group_requests", {{"from_user_id", from_user_id},
                                             {"to_user_id", admin_id},
                                             {"group_id", group_id},
                                             {"json", js.dump()}});
        }
    }
}

void GroupAddAcker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
}

void GroupLister::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string user_id = js["user_id"];
    sendGroupList(user_id);
}

void GroupLister::sendGroupList(std::string &user_id)
{
    auto conn = service_->getConnectionPtr(user_id);
    if (conn == nullptr)
        return;
    auto groupsId = getGroupsId(user_id);
    auto groups = getGroupsInfo(groupsId);

    json groupList;
    groupList["msgid"] = GET_GROUPS;
    for (const auto &group : groups)
    {
        json g;
        g["id"] = group.at("id");
        g["name"] = group.at("name");
        groupList["groups"].push_back(g);
    }
    sendJson(conn, groupList);
}

Result GroupLister::getGroupsId(std::string &user_id)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    return mysql->select("group_members", {{"user_id", user_id}});
}

Result GroupLister::getGroupsInfo(Result &groupsId)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    std::vector<std::string> id_list;
    for (const auto &group_map : groupsId)
    {
        id_list.push_back(group_map.at("group_id"));
    }
    return mysql->select("`groups`", {}, {{"id", id_list}});
}