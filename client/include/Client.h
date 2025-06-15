#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include "base.h"

#include "Neter.h"
#include "UserService.h"
#include "ChatService.h"
#include "FriendService.h"
#include "GroupService.h"
//#include "FileService.h"

#include "Controller.h"

class Client
{
    friend class Controller;
    friend class UserService;
    friend class ChatService;
    friend class FriendService;
    friend class GroupService;
    friend class FileService;

public:
    Client();
    void start();

    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void handleJson(const TcpConnectionPtr &conn, const std::string &jsonStr);
    void logicLoop();

    std::string user_id_;
    std::string user_email_;
    std::vector<Friend> friendList_;
    std::vector<Group> groupList_;
    std::unordered_map<std::string, ChatLogs> chatLogs_;        // map< freind_id,vector<log> >
    std::unordered_map<std::string, ChatLogs> groupChatLogs_;   //   map<  group_id,vector<log>  >
    std::unordered_map<std::string, std::string> unreadCounts_; // map< friend_id,unreadCount >
    std::vector<FriendRequest> friendRequests_;
    std::vector<GroupAddRequest> groupAddRequests_;

    Friend currentFriend_;
    Group currentGroup_;

private:
    std::thread logicThread_;
    Neter neter_;

    UserService userService_;
    ChatService chatService_;
    FriendService friendService_;
    GroupService groupService_;
    //FileService fileService_;

    Controller controller_;
};