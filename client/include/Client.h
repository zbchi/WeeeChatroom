#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include <unordered_map>
#include "base.h"

#include "Neter.h"
#include "UserService.h"
#include "ChatService.h"
#include "FriendService.h"
#include "GroupService.h"
#include "FileService.h"

#include "Controller.h"

using FriendChatLogs = std::unordered_map<std::string, ChatLogs>;
using GroupChatLogs = std::unordered_map<std::string, ChatLogs>;
using FriendRequests = std::vector<FriendRequest>;
using GroupAddRequests = std::vector<GroupAddRequest>;
using FriendList = std::unordered_map<std::string, Friend>;
using GroupList = std::unordered_map<std::string, Group>;
using FileList = std::vector<FileInfo>;
using MsgHandler = std::function<void(const TcpConnectionPtr &, json &)>;
using MsgHanlerMap = std::unordered_map<int, MsgHandler>;

using IsReadMap = std::unordered_map<std::string, bool>; // true为未读

class Client
{
    friend class Controller;
    friend class UserService;
    friend class ChatService;
    friend class FriendService;
    friend class GroupService;
    friend class FileService;
    friend class FtpClient;

public:
    Client(const char *serverAddr);
    void start();

    MsgHanlerMap msgHandlerMap_;

    void handleJson(const TcpConnectionPtr &conn, const std::string &jsonStr);

    std::string user_id_;
    std::string user_email_;
    std::string nickname_;

    FriendList friendList_;
    std::mutex friendListMutex_;

    GroupList groupList_;
    std::mutex groupListMutex_;

    FileList fileList_;
    FriendChatLogs chatLogs_;     // map< freind_id,vector<log> >
    GroupChatLogs groupChatLogs_; //   map<  group_id,vector<log>  >
    FriendRequests friendRequests_;
    GroupAddRequests groupAddRequests_;

    Friend currentFriend_;
    Group currentGroup_;

    IsReadMap isReadMap_;
    IsReadMap isReadGroupMap_;
    std::mutex isReadMapMutex_;
    std::mutex isReadGroupMapMutex_;

private:
    std::string serverAddr_;
    Neter neter_;

    UserService userService_;
    ChatService chatService_;
    FriendService friendService_;
    GroupService groupService_;
    FileService fileService_;

    Controller controller_;
};