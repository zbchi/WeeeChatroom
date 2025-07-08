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
class FileService
{
public:
    FileService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}

    void uploadFile(std::string &filePath, bool is_group);

private:
    FtpClient ftpClient_;
    Neter *neter_;
    Client *client_;
};