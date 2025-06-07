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
    auto admins = mysql->select("group_members", {{"group_id", group_id}},
                                {{"role", {"admin", "owner"}}});

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
    // 处理加群请求的回应
    std::string response = js["response"];
    std::string from_user_id = js["from_user_id"];
    std::string group_id = js["group_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    if (response == "accept")
    {
        mysql->insert("group_members", {{"group_id", group_id},
                                        {"user_id", from_user_id}});
    }
    else if (response == "reject")
    {
        std::cout << "rejectrejectrejectrejectrejectrejectreject" << std::endl;
    }

    // 更新用户群列表
    GroupLister list(service_);
    list.sendGroupList(from_user_id);

    // 广播给所有管理员
    json notice;
    notice["msgid"] = ADD_GROUP_REMOVE;
    notice["from_user_id"] = from_user_id;
    notice["group_id"] = group_id;

    auto admins = mysql->select("group_members", {{"group_id", group_id}},
                                {{"role", {"admin", "owner"}}});

    for (const auto &admin : admins)
    {
        std::string admin_id = admin.at("user_id");
        auto targetConn = service_->getConnectionPtr(admin_id);
        if (targetConn != nullptr) // 在线直接转发
        {
            sendJson(targetConn, notice);
        }
        else // 离线存储加remove的json通知，待管理员上线后发
        {
            notice["to_user_id"] = admin_id;
            mysql->insert("remove_jsons", {{"to_user_id", admin_id},
                                           {"json", notice.dump()}});
        }
    }
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
    if (groupsId.empty())
        return {};
    auto mysql = MySQLConnPool::instance().getConnection();
    std::vector<std::string> id_list;
    for (const auto &group_map : groupsId)
    {
        id_list.push_back(group_map.at("group_id"));
    }
    return mysql->select("`groups`", {}, {{"id", id_list}});
}

void GroupInfoSender::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string group_id = js["group_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    auto membersId = mysql->select("group_members", {{"group_id", group_id}});

    json membersInfo;
    membersInfo["msgid"] = GET_GROUPINFO;

    for (const auto &memberId : membersId)
    {
        json m;
        std::string user_id = memberId.at("user_id");
        m["user_id"] = user_id;
        m["role"] = memberId.at("role");
        auto member_info = mysql->select("users", {{"id", user_id}});
        m["nickname"] = member_info[0].at("nickname");

        membersInfo["members"].push_back(m);
    }
    sendJson(conn, membersInfo);
}

void GroupExiter::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string group_id = js["group_id"];
    std::string user_id = js["user_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("group_members", {{"group_id", group_id},
                                                  {"user_id", user_id}});

    std::string role = result[0].at("role");
    if (role == "member" || role == "admin")
    {
        mysql->del("group_members", {{"group_id", group_id},
                                     {"user_id", user_id}});
    }
    else if (role == "owner")
    { // 如果解散，更新群成员的群列表
        auto member_ids = mysql->select("group_members", {{"group_id", group_id}});
        GroupLister list(service_);
        for (auto &member_id : member_ids)
            list.sendGroupList(member_id.at("user_id"));
        // 删库跑路
        mysql->del("group_members", {{"group_id", group_id}});
        mysql->del("group_messages", {{"group_id", group_id}});
        mysql->del("`groups`", {{"id", group_id}});
    }
}

void MemberKicker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string kick_user_id = js["kick_user_id"];
    std::string user_id = js["user_id"];
    std::string group_id = js["group_id"];
    auto mysql = MySQLConnPool::instance().getConnection();
    auto result = mysql->select("group_members", {{"user_id", user_id},
                                                  {"group_id", group_id}});
    std::string role = result[0].at("role");
    if (role != "member")
    {
        mysql->del("group_members", {{"group_id", group_id},
                                     {"user_id", kick_user_id}});
        // 更新被ti的群列表
        GroupLister list(service_);
        list.sendGroupList(kick_user_id);
    }
}