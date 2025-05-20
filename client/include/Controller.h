#pragma once
#include <map>
#include <Handler.h>

enum class State
{
    INIT,
    REGISTERING,
    LOGINING,
    LOGGED_IN,
    SHOW_FREINDS,
    SHOW_GROUPS,
    CHAT_IN,

};
extern State state_;
class Client;
class Neter;

class Controller
{
public:
    Controller(Client *client, Neter *neter) : client_(client), neter_(neter) {}
    void mainLoop();

private:
    void showRegister();
    void showLogin();
    void showMenue();
    Client *client_;
    Neter *neter_;
};