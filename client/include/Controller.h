#pragma once
#include <map>
#include <Handler.h>
#include <mutex>
#include <condition_variable>
#include "base.h"
enum class State
{
    INIT,
    REGISTERING,
    LOGINING,
    MAIN_MENU,
    CHAT_PANEL,
    SHOW_FREINDS,
    SHOW_GROUPS,
    CHAT_FRIEND,
    CHAT_GROUP,
    ADD_FRIEND,
    DEL_FRIEND,
    HANDLE_FRIEND_REQUEST,
    CREATE_GROUP,
    ADD_GROUP,
    HANDLE_GROUP_REQUEST,
    SHOW_MEMBERS,
    EXIT_GROUP,
    DESTORY_GROUP,
    ADD_ADMIN,
};
extern State state_;
class Client;
class Neter;

class Controller
{
public:
    Controller(Neter *neter, Client *client) : client_(client), neter_(neter) {}
    void mainLoop();

    

    void flushLogs();
    void flushGroupLogs();
    void flushFriends();
    void flushRequests();
    void flushGroupRequests();
    void flushGroups();

private:
    void showMainMenu();
    void showChatPanel();
    void showLogin();
    void showRegister();
    void showFriends();
    void chatWithFriend();
    void chatWithGroup();
    void showAddFriend();
    void showDelFriend();
    void showHandleFriendRequest();
    void showCreateGroup();
    void showAddGroup();
    void showHandleGroupRequest();
    void showGroups();
    void showGroupMembers();
    void showExitGroup();
    void showDestroyGroup();

    void showGroupMenu();
    void showFriendMenu();
    void showSystemMenu();
    Client *client_;
    Neter *neter_;
};