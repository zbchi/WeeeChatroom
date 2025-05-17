#pragma once
#include <map>
#include <Handler.h>
class Client;
class Neter;
class Controller
{
public:
    Controller(Client *client, Neter *neter) : client_(client), neter_(neter) {}
    void mainLoop();

private:
    Client *client_;
    Neter *neter_;
};