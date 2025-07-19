#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"
#include "Sendfile.h"
#include "ui.h"
#include <unistd.h>

FtpClient::FtpClient(const FileInfo &fileinfo, Client *client) : client_(client),
                                                                 serverAddr_(client_->serverAddr_.c_str(), 8001),
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
    { // 连接成功后自动传输信息
        if (fileInfo_.is_upload)
            sendUploadInfo(conn);
        else
            sendDownloadInfo(conn);
    }
    else
    {
        printTopBegin();
        std::cout << "文件[" << fileInfo_.file_name << "]传输完成";
        printTopEnd();
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

    std::string content = "[文件]:" + fileInfo_.file_name;
    if (fileInfo_.is_group)
        client_->chatService_.sendGroupMessage(content);
    else
        client_->chatService_.sendMessage(content);
}

void FtpClient::sendDownloadInfo(const TcpConnectionPtr &conn)
{
    json js;
    js["msgid"] = DOWNLOAD_FILE;
    js["file_id"] = fileInfo_.id;
    std::string file_path = makeFilePath(fileInfo_.file_name);
    off_t file_size = static_cast<off_t>(std::stoll(fileInfo_.file_size_str));
    js["file_size"] = file_size;
    int file_fd = ::open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    auto ctx = std::make_shared<FileContext>(file_fd, std::stoi(fileInfo_.id), 0, file_size);
    ::pipe(ctx->pipefd);
    conn->setContext(ctx);
    conn->setReadableCallback([this](const TcpConnectionPtr &conn)
                              { recvFileData(conn); });
    conn->send(js.dump());
}

std::string FtpClient::makeFilePath(const std::string &file_name)
{
    fs::path file_dir = "/tmp/chatclient/chat_files";
    std::string path = (file_dir / file_name).string();
    fs::create_directories(file_dir);
    return path;
}

void FtpClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    if (!conn->getContext().has_value())
    {
        std::string jsonStr(buf->peek(), buf->readableBytes());
        json js = json::parse(jsonStr);
        int msgid = js["msgid"].get<int>();
        if (msgid == UPLOAD_FILE_ACK)
            sendFile(conn, fileInfo_.fileFd, 0, fileInfo_.file_size);
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
    fileInfo.is_upload = true;
    ftpClientManager_.uploadFile(fileInfo);
}

void FileService::downloadFile(FileInfo &fileinfo)
{
    fileinfo.is_upload = false;
    ftpClientManager_.donwloadFile(fileinfo);
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

void FileService::handleFileList(const TcpConnectionPtr &conn, json &js)
{
    client_->fileList_.clear();
    FileInfo f;
    for (const auto &afile : js["files"])
    {
        f.file_name = afile["file_name"];
        f.file_size_str = afile["file_size_str"];
        f.timestamp = afile["timestamp"];
        f.sender_id = afile["sender_id"];
        f.id = afile["id"];
        client_->fileList_.push_back(f);
    }
    fileListWaiter_.notify(0);
}
