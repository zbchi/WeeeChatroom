#pragma once
#include <string>
#include <mutex>
#include "Handler.h"
#include "base.h"
class Neter;
class Client;
class FileService
{
public:
    FileService(Neter *neter, Client *client) : neter_(neter),
                                                client_(client) {}
    
private:
    Neter *neter_;
    Client *client_;
};