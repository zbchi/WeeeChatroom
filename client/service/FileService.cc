#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

#include <filesystem>
#include <unistd.h>
namespace fs = std::filesystem;

FtpClient::FtpClient(const std::string &file_path, const std::string &file_id, bool is_upload) : serverAddr_("127.0.0.1", 8001),
                                                                                                 tcpClient_(&loop_, serverAddr_)
{
    tcpClient_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                     { if(conn->connected())
                                {
                                   
                                }
                                else
                                 {
                                   loop_.quit();
                                   LOG_DEBUG("Loop Quit!");
                                 } });
    tcpClient_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                  { this->onMessage(conn, buf, time); });
    tcpClient_.connect();
    loop_.loop();
}

void FtpClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
}

void FileService::uploadFile(std::string &filePath, bool is_group)
{

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

    ftpClientManager_.uploadFile(filePath, std::to_string(file_id));
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