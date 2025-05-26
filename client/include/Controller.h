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
    CHAT_FRIEND,

};
extern State state_;
class Client;
class Neter;

class Controller
{
public:
    Controller(Neter *neter, Client *client) : client_(client), neter_(neter) {}
    void mainLoop();

private:
    void showRegister();
    void showLogin();
    void showMenue();
    void showFriends();
    void chatWithFriend();
    Client *client_;
    Neter *neter_;
};