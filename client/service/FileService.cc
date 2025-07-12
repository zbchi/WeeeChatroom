#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"
#include "Sendfile.h"

#include <unistd.h>

FtpClient::FtpClient(const FileInfo &fileinfo) : serverAddr_("127.0.0.1", 8001),
                                                 tcpClient_(&loop_, serverAddr_),
                                                 fileInfo_(fileinfo)
{
    tcpClient_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                     { this->onConnection(conn); });
    tcpClient_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                  { this->onMessage(conn, buf, time); });
    tcpClient_.connect();
    loop_.loop();
}

void FtpClient::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    { // 连接成功后自动传输文件
        if (fileInfo_.is_upload)
        {
            sendUploadInfo(conn);
        }
        else
        {
        }
    }
    else
    {
        loop_.quit();
        LOG_DEBUG("Loop Quit!");
    }
}

void FtpClient::sendUploadInfo(const TcpConnectionPtr &conn)
{
    json js;
    js["msgid"] = UPLOAD_FILE;
    js["is_group"] = fileInfo_.is_group;
    js["sender_id"] = fileInfo_.sender_id;
    js["peer_id"] = fileInfo_.peer_id;
    js["file_name"] = fileInfo_.file_name;
    int fd = ::open(fileInfo_.file_path.c_str(), O_RDONLY);
    fileInfo_.fileFd = fd;
    fileInfo_.file_size = getFileSize(fd);
    js["file_size"] = fileInfo_.file_size;
    conn->send(js.dump());
}

void FtpClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    if (!conn->getContext().has_value())
    {
        std::string jsonStr(buf->peek(), buf->readableBytes());
        json js = json::parse(jsonStr);
        int msgid = js["msgid"].get<int>();
        if (msgid == UPLOAD_FILE_ACK)
        {
            sendFile(conn, fileInfo_.fileFd, 0, fileInfo_.file_size);
        }
        else if(msgid==DOWNLOAD_FILE_ACK)
        {
            
        }
    }
}

void FileService::uploadFile(std::string &filePath, bool is_group)
{

    json js;
    fs::path p(filePath.c_str());

    FileInfo fileInfo;
    fileInfo.file_path = filePath;
    fileInfo.is_group = is_group;
    fileInfo.sender_id = client_->user_id_;
    fileInfo.peer_id = is_group ? client_->currentGroup_.group_id_ : client_->currentFriend_.id_;
    fileInfo.file_name = extractFilename(filePath);

    // uploadWaiter_.wait();
    // int file_id = uploadWaiter_.getResult();

    ftpClientManager_.uploadFile(fileInfo);
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
}