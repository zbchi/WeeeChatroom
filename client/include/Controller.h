#pragma once
#include <map>
#include <Handler.h>
#include <mutex>
#include <condition_variable>

enum class State
{
    INIT,
    REGISTERING,
    LOGINING,
    LOGGED_IN,
    SHOW_FREINDS,
    SHOW_GROUPS,
    CHAT_FRIEND,
    ADD_FRIEND,
    DEL_FRIEND,
    HANDLE_FRIEND_REQUEST,
};
extern State state_;
class Client;
class Neter;

class Controller
{
public:
    Controller(Neter *neter, Client *client) : client_(client), neter_(neter) {}
    void mainLoop();

    int login_errno_ = -1;
    int reg_errno_ = -1;
    std::mutex login_mtx_;
    std::mutex reg_mtx_;
    std::condition_variable loginCv_;
    std::condition_variable regCv_;
    bool loginResultSet_ = false;
    bool regResultSet_ = false;

    void flushLogs();
    void flushRequests();

private:
    void showRegister();
    void showLogin();
    void showMenue();
    void showFriends();
    void chatWithFriend();
    void showAddFriend();
    void showDelFriend();
    void showHandleFriendRequest();
    Client *client_;
    Neter *neter_;
};