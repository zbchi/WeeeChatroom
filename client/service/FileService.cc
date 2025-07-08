#include "FileService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

#include <filesystem>
namespace fs = std::filesystem;

void FileService::uploadFile(std::string &filePath, bool is_group = false)
{
    json js;
    fs::path p(filePath.c_str());
    js["msgid"] = UPLOAD_FILE;
    js["is_group"] = is_group;
    js["sender_id"] = client_->user_id_;
    js["peer_id"] = is_group ? client_->currentGroup_.group_id_ : client_->currentFriend_.id_;
    js["file_name"] =p.filename().string();
    js["file_size"]=fs::file_size(filePath.c_str());

    ftpClient_.uploadFile(filePath);
}