#include "File.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

void FileUploader::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    bool is_group = js["is_group"];
    std::string sender_id = js["sender_id"];
    std::string peer_id = js["peer_id"];
    std::string file_name = js["file_name"];
    std::uint64_t file_size = js["file_size"];
    auto mysql = MySQLConnPool::instance().getConnection();

    mysql->insert("files", {{"sender_id", sender_id},
                            {is_group ? "group_id" : "receiver_id", peer_id},
                            {"file_size", std::to_string(file_size)},
                            {"file_name", file_name},
                            {"is_group", is_group ? "1" : "0"}});
}

void FileLister::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    bool is_group = js["is_group"];
    std::string user_id = js["user_id"];
    std::string peer_id = js["peer_id"];
    auto mysql = MySQLConnPool::instance().getConnection();

    Result files;
    if (is_group)
        files = mysql->select("files", {{"group_id", peer_id}});
    else
        files = mysql->select("files", {{"sender_id", peer_id},
                                        {"receiver_id", user_id}});

    json fileList;
    fileList["msgid"] = GET_FILES;
    for (const auto &file : files)
    {
        json f;
        f["id"] = file.at("id");
        f["file_name"] = file.at("file_name");
        f["file_size"] = file.at("file_size");
        f["timestamp"] = file.at("send_at");
        fileList["files"].push_back(f);
    }

    sendJson(conn, fileList);
}