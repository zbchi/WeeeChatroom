#include "File.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Sendfile.h"

FtpServer::FtpServer() : listenAddr_(8001),
                         tcpServer_(&loop_, listenAddr_) {}

void FtpServer::start()
{
    tcpServer_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                     { this->onConnection(conn); });
    tcpServer_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                  { this->onMessage(conn, buf, time); });
    tcpServer_.setThreadNum(4);
    tcpServer_.start();
    loop_.loop();
}

void FtpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
    }
    else
        std::cout << "ondisconnectedondisconnected" << std::endl;
}

void FtpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    // auto context = std::any_cast_or_null<std::shared_ptr<FileContext>>(conn->getMutableContext());
    if (!conn->getContext().has_value())
    {
        std::string jsonStr(buf->peek(), buf->readableBytes());
        std::cout << jsonStr << std::endl;
        json js = json::parse(jsonStr);
        int msgid = js["msgid"].get<int>();
        if (msgid == UPLOAD_FILE)
        {
            // 做好接收文件的前提准备(文件信息写入数据库，创建文件fd，写入conn上下文，设置可读回调收文件)
            bool is_group = js["is_group"];
            std::string sender_id = js["sender_id"];
            std::string peer_id = js["peer_id"];
            std::string file_name = js["file_name"];
            off_t file_size = js["file_size"];
            auto mysql = MySQLConnPool::instance().getConnection();

            mysql->insert("files", {{"sender_id", sender_id},
                                    {is_group ? "group_id" : "receiver_id", peer_id},
                                    {"file_size", std::to_string(file_size)},
                                    {"file_name", file_name},
                                    {"is_group", is_group ? "1" : "0"}});

            int file_id = mysql->getLastInsertId();
            std::string file_path = makeFilePath(std::to_string(file_id));

            int file_fd = ::open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            auto ctx = std::make_shared<FileContext>(file_fd, file_id, 0, file_size);
            ::pipe(ctx->pipefd);
            conn->setContext(ctx);
            conn->setReadableCallback([this](const TcpConnectionPtr &conn)
                                      { recvFileData(conn); });
            // 向客户端发送ack接收文件准备完成
            conn->send(makeResponse(UPLOAD_FILE_ACK, 0).dump());
            LOG_INFO("开始接收文件[%d]", file_id);
        }
        else if (msgid == DOWNLOAD_FILE)
        {
            std::string file_id = js["file_id"];
            off_t file_size = js["file_size"];
            std::string file_path = makeFilePath(file_id);
            int file_fd = ::open(file_path.c_str(), O_RDONLY);
            LOG_INFO("开始发送文件[%s]", file_id.c_str());
            sendFile(conn, file_fd, 0, file_size, std::stoi(file_id));
        }
    }
    else
    {
        auto ctx = std::any_cast<std::shared_ptr<FileContext>>(conn->getContext());
        int written = ::write(ctx->fileFd, buf->peek(), buf->readableBytes());
        buf->retrieve(written);
        ctx->written += written;
        if (ctx->written >= ctx->totalSize)
        {
            ::close(ctx->fileFd);
            conn->setContext(std::any());
            conn->shutdown();
        }
    }
}

void FileUploader::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
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
        f["file_size_str"] = file.at("file_size");
        f["timestamp"] = file.at("send_at");
        f["sender_id"] = file.at("sender_id");
        fileList["files"].push_back(f);
    }

    sendJson(conn, fileList);
}

std::string FtpServer::makeFilePath(const std::string &file_id)
{
    fs::path exe_path = fs::canonical("/proc/self/exe");
    fs::path exe_dir = exe_path.parent_path();
    fs::path file_dir = exe_dir / "chat_files";
    std::string path = (file_dir / file_id).string();
    fs::create_directories(fs::path(path).parent_path());
    return path;
}