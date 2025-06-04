#pragma once
#include <string>
#include "Handler.h"

class Neter;
class Client;
class GroupService
{
public:
    GroupService(Neter *neter, Client *client) : neter_(neter),
                                                 client_(client) {}

    void createGroup(std::string &groupname, std::string &description);

private:
    Neter *neter_;
    Client *client_;
};
