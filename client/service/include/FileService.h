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
    FtpClient(EventLoop *loop);

    TcpClient tcpClient_;
    InetAddress serverAddr_;

    void uploadFile(const std::string &file_path, const std::string &file_id);
    void downloadFile(const std::string &file_id);

    std::mutex connMutex_;
    std::condition_variable connCond_;
    mylib::TcpConnectionPtr conn_;

    using DisConnectedCallback = std::function<void()>;
    DisConnectedCallback onDisconnectedCallback_;
    void setDisconnectedCallback(const DisConnectedCallback &cb)
    {
        onDisconnectedCallback_ = cb;
    }

private:
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void onDisConnected()
    {
        if (onDisconnectedCallback_)
            onDisconnectedCallback_(); // 通知外部销毁自己
    }
};

class FtpClientThread : public std::enable_shared_from_this<FtpClientThread>
{
public:
    void start();

    void uploadFile(const std::string &file_path, const std::string &file_id)
    {
        loop_->runInLoop([this, file_path, file_id]
                         { ftpClient_->uploadFile(file_path, file_id); });
    }
    void downloadFile(const std::string &file_id)
    {
        loop_->runInLoop([this, file_id]
                         { ftpClient_->downloadFile(file_id); });
    }
    void stop()
    {
        loop_->runInLoop([this]()
                         {
                ftpClient_->tcpClient_.disconnect();
                loop_->quit(); });
    }

private:
    EventLoopThread loopThread_;
    std::thread thread_;
    EventLoop *loop_;
    std::unique_ptr<FtpClient> ftpClient_;
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

private:
    Neter *neter_;
    Client *client_;
};
