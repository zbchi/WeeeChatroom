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
    if (is_group)
    {
        mysql->insert("files", {{"sender_id", sender_id},
                                {"group_id", peer_id},
                                {"file_size", std::to_string(file_size)},
                                {"file_name", file_name},
                                {"is_group", is_group ? "1" : "0"}});
    }
    else
    {
        mysql->insert("files", {{"sender_id", sender_id},
                                {"receiver_id", peer_id},
                                {"file_size", std::to_string(file_size)},
                                {"file_name", file_name},
                                {"is_group", is_group ? "1" : "0"}});
    }
}