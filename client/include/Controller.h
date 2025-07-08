#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include "base.h"
#include "ChatService.h"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define GRADIENT_START "\033[38;5;45m"
#define GRADIENT_END "\033[38;5;81m"
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
    DEL_FRIEND,
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
};
extern State state_;

class Client;
class Neter;

class Controller
{
public:
    Controller(Neter *neter, Client *client) : client_(client), neter_(neter) {}
    void mainLoop();

    void printLogs(ChatLogs &chatLogs,bool is_group=false);
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
    void showFindPassword();
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

    void fileWithFriend();
    void fileWithGroup();
    Client *client_;
    Neter *neter_;
};