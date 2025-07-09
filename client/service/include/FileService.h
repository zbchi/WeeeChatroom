#pragma once
#include <string>
#include "base.h"
class Neter;
class Client;
class FtpClient
{
public:
    void uploadFile(std::string &filePath);
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

    void getFiles(bool is_group=false);
    void handleFileList(const TcpConnectionPtr&conn,json&js);
    void uploadFile(std::string &filePath, bool is_group = false);

    Waiter fileListWaiter_;

private:
    FtpClient ftpClient_;
    Neter *neter_;
    Client *client_;
};

