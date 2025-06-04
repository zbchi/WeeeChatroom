#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <thread>

#include "Neter.h"
#include "UserService.h"
#include "ChatService.h"
#include "FriendService.h"
#include "GroupService.h"

#include "MessageQueue.h"

#include "Controller.h"
using namespace mylib;
using json = nlohmann::json;

class Client
{
    friend class Controller;
    friend class UserService;
    friend class ChatService;
    friend class FriendService;

public:
    Client();
    void start();

    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    MessageQueue messageQueue_;

    void handleJson(const TcpConnectionPtr &conn, const std::string &jsonStr);
    void logicLoop();

    std::string user_id_;
    std::string user_email_;
    std::vector<Friend> friendList_;
    std::unordered_map<std::string, ChatLog> chatLogs_;         // map< freind_id,vector<log> >
    std::unordered_map<std::string, std::string> unreadCounts_; // map< friend_id,unreadCount >
    std::vector<FriendRequest> friendRequests_;

    Friend currentFriend_;

private:
    std::thread logicThread_;
    Neter neter_;
    UserService userService_;
    ChatService chatService_;
    FriendService friendService_;
    GroupService groupService_;

    Controller controller_;
};