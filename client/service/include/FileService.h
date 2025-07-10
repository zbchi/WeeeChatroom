#pragma once
#include <string>
#include "base.h"
#include "TcpClient.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

class Neter;
class Client;
class FileService;
class FtpClient
{
public:
    FtpClient(const std::string &file_path, const std::string &file_id, bool is_upload);

    void uploadFile(const std::string &file_path, const std::string &file_id);
    void downloadFile(const std::string &file_id);

private:
    std::string file_path;
    std::string file_id;
    bool is_upload;

    InetAddress serverAddr_;
    EventLoop loop_;
    TcpClient tcpClient_; // 构造顺严格执行，为声明的顺序
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void onConnection(const TcpConnectionPtr &conn);
};

class FtpClientManager
{
public:
    void uploadFile(const std::string &file_path, const std::string &file_id)
    {
        std::thread([file_path, file_id]
                    { auto ftpClient = std::make_unique<FtpClient>(file_path, file_id, true); 
                    LOG_DEBUG("Thread Die!"); })
            .detach();
    }
    void donwloadFile(const std::string &file_id)
    {
        std::thread([file_id]
                    { auto ftpClient = std::make_unique<FtpClient>("", file_id, false);
                     LOG_DEBUG("Thread Die!"); })
            .detach();
    }
};

class File
{
public:
    std::string id;
    std::string file_name;
    std::string file_size;

    std::string timestamp;
};

class FileService
{
public:
    FileService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}

    void getFiles(bool is_group = false);
    void handleFileList(const TcpConnectionPtr &conn, json &js);
    void uploadFile(std::string &filePath, bool is_group = false);
    void handleUploadAck(const TcpConnectionPtr &conn, json &js);
    Waiter fileListWaiter_;
    Waiter uploadWaiter_;

    FtpClientManager ftpClientManager_;

private:
    Neter *neter_;
    Client *client_;
};
