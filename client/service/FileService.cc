#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

#include <filesystem>
namespace fs = std::filesystem;

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
    ftpClient_.uploadFile(filePath);
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
    File f;
    for (const auto &afile : js["files"])
    {
        f.file_name = afile["file_name"];
        f.file_size = afile["file_size"];
        f.id = afile["id"];
        client_->fileList_.push_back(f);
    }
    fileListWaiter_.notify(0);
}

void FtpClient::uploadFile(std::string &filePath)
{
}