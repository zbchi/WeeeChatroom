#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include "base.h"
#include "ChatService.h"
#include "ThreadPool.h"
#include <atomic>

enum class State
{
    INIT,
    REGISTERING,
    LOGINING,
    MAIN_MENU,
    FRIEND_MENU,
    GROUP_MENU,
    CHAT_PANEL,
    SHOW_FREINDS,
    SHOW_GROUPS,
    CHAT_FRIEND,
    CHAT_GROUP,
    ADD_FRIEND,

    HANDLE_FRIEND_REQUEST,
    CREATE_GROUP,
    ADD_GROUP,
    HANDLE_GROUP_REQUEST,
    SHOW_MEMBERS,
    EXIT_GROUP,
    DESTORY_GROUP,
    ADD_ADMIN,
    FIND_PASSWORD,
    FILE_FRIEND,
    FILE_GROUP,

    FRIEND_PANEL,
    GROUP_PANEL,

    DESTROY_ACCOUNT,
    LOG_OR_REG,

    LOG_HISTORY
};
extern std::atomic<State> state_;

class Client;
class Neter;

class Controller
{
public:
    Controller(Neter *neter, Client *client) : client_(client), neter_(neter), pool_(1) {}
    void mainLoop();

    void printLogs(const ChatLogs &chatLogs, bool is_group = false);
    void flushLogs();
    void flushGroupLogs();
    void flushRequests();
    void flushGroupRequests();
    void flushFriends();
    void flushGroups();
    void flushFiles(bool is_group = false);
    void printALog(const ChatMessage &log, bool is_group);

private:
    void showMainMenu();
    void showChatPanel();
    void showLogin();
    void showRegister();
    void showFindPassword();
    void chatWithFriend();
    void chatWithGroup();
    void showAddFriend();

    void showHandleFriendRequest();
    void showCreateGroup();
    void showAddGroup();
    void showHandleGroupRequest();
    void showGroupMembers();
    void showDestroyGroup();

    void showGroupMenu();
    void showFriendMenu();
    void showSystemMenu();

    void friendPanel();
    void groupPanel();

    void filePanel(bool is_group = false);

    void showDestroyAccount();

    void showLogOrReg();

    std::mutex printMutex_;
    Client *client_;
    Neter *neter_;
    ThreadPool pool_;
};