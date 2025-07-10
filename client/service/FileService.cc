#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

#include <filesystem>
#include <unistd.h>
namespace fs = std::filesystem;

void FtpClientThread::start()
{
    loop_ = loopThread_.startLoop();
    std::weak_ptr<FtpClientThread> weakSelf = shared_from_this();
    loop_->runInLoop([this, weakSelf]()
                     { 
                        ftpClient_ = std::make_unique<FtpClient>(loop_);
                        ftpClient_->setDisconnectedCallback([weakSelf]()
                        {
                            if(auto self=weakSelf.lock())
                            self->stop();
                        }) ; });
}

FtpClient::FtpClient(EventLoop *loop) : serverAddr_("127.0.0.1", 8001),
                                        tcpClient_(loop, serverAddr_)
{
    tcpClient_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                     { if(conn->connected())
                                {

                                }
                                else
                                 {
                                    // 连接断开或连接失败
                                   // std::lock_guard<std::mutex> lock(connMutex_);
                                    //conn_.reset();
                                    //断开连接回调销毁自己
                                   // onDisConnected();
                                 }
                                connCond_.notify_one(); });
    tcpClient_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                  { this->onMessage(conn, buf, time); });
    tcpClient_.connect();
}

void FtpClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
}

void FileService::uploadFile(std::string &filePath, bool is_group)
{
    auto ftpThread = std::make_shared<FtpClientThread>();
    ftpThread->start();

    json js;
    fs::path p(filePath.c_str());
    js["msgid"] = UPLOAD_FILE;
    js["is_group"] = is_group;
    js["sender_id"] = client_->user_id_;
    js["peer_id"] = is_group ? client_->currentGroup_.group_id_ : client_->currentFriend_.id_;
    js["file_name"] = p.filename().string();
    js["file_size"] = fs::file_size(filePath.c_str());
    client_->neter_.sendJson(js);
    uploadWaiter_.wait();
    int file_id = uploadWaiter_.getResult();

    ftpThread->uploadFile(filePath, std::to_string(file_id));
    // ftpClient_.uploadFile(filePath, std::to_string(file_id));
    sleep(10);
}

void FileService::getFiles(bool is_group)
{
    json getInfo;
    getInfo["msgid"] = GET_FILES;
    getInfo["user_id"] = client_->user_id_;
    getInfo["peer_id"] = is_group ? client_->currentGroup_.group_id_ : client_->currentFriend_.id_;
    getInfo["is_group"] = is_group;
    neter_->sendJson(getInfo);
    fileListWaiter_.wait();
}

void FileService::handleUploadAck(const TcpConnectionPtr &conn, json &js)
{
    int file_id = js["file_id"];
    uploadWaiter_.notify(file_id);
}

void FileService::handleFileList(const TcpConnectionPtr &conn, json &js)
{
    client_->fileList_.clear();
    File f;
    for (const auto &afile : js["files"])
    {
        f.file_name = afile["file_name"];
        f.file_size = afile["file_size"];
        f.timestamp = afile["timestamp"];
        f.id = afile["id"];
        client_->fileList_.push_back(f);
    }
    fileListWaiter_.notify(0);
}

void FtpClient::uploadFile(const std::string &filePath, const std::string &file_id)
{
    std::cout << "upload  ------------------------------" << std::endl;
}