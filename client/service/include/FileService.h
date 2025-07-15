#pragma once
#include <string>
#include "base.h"
#include "TcpClient.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

class Neter;
class Client;
class FileService;

struct FileInfo
{
    std::string file_path;
    std::string id;
    bool is_upload;
    bool is_group;
    off_t file_size;
    int fileFd;

    std::string sender_id;
    std::string peer_id;
    std::string file_name;
    std::string file_size_str;
    std::string timestamp;
};

class FtpClient
{
public:
    FtpClient(const FileInfo &fileinfo, Client *client);
    void downloadFile(const std::string &file_id);

private:
    void sendUploadInfo(const TcpConnectionPtr &conn);
    void sendDownloadInfo(const TcpConnectionPtr &conn);

    std::string makeFilePath(const std::string &file_name);
    FileInfo fileInfo_;

    InetAddress serverAddr_;
    EventLoop loop_;
    TcpClient tcpClient_; // 构造顺严格执行，为声明的顺序
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void onConnection(const TcpConnectionPtr &conn);

    Client *client_;
};

class FtpClientManager
{
public:
    FtpClientManager(Client *client) : client_(client) {}
    void uploadFile(FileInfo &fileInfo)
    {
        std::thread([fileInfo, this]
                    { auto ftpClient = std::make_unique<FtpClient>(fileInfo,client_); 
                    LOG_DEBUG("Thread Die!"); })
            .detach();
    }
    void donwloadFile(FileInfo &fileInfo)
    {
        std::thread([fileInfo, this]
                    { auto ftpClient = std::make_unique<FtpClient>(fileInfo,client_);
                     LOG_DEBUG("Thread Die!"); })
            .detach();
    }

private:
    Client *client_;
};

class FileService
{
public:
    FileService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client),
                                                ftpClientManager_(client) {}

    void getFiles(bool is_group = false);
    void handleFileList(const TcpConnectionPtr &conn, json &js);

    void uploadFile(std::string &filePath, bool is_group = false);
    void downloadFile(FileInfo &fileinfo);
    Waiter fileListWaiter_;

    FtpClientManager ftpClientManager_;

private:
    Neter *neter_;
    Client *client_;
};
