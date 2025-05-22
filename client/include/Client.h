#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <thread>

#include "Neter.h"
#include "UserService.h"
#include "ChatService.h"
#include "MessageQueue.h"
#include "Handler.h"
#include "Controller.h"
using namespace mylib;
using json = nlohmann::json;

class Friend
{
public:
    std::string id_;
    std::string nickname_;
    bool isOnline_;
    void setCurrentFriend(Friend &friendObj)
    {
        id_ = friendObj.id_;
        nickname_ = friendObj.nickname_;
        isOnline_ = friendObj.isOnline_;
    }
};

class Client
{
    friend class Controller;

public:
    Client();
    void start();

    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;
    MessageQueue messageQueue_;
    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr);

    std::string user_id_;
    std::vector<Friend> firendList_;
    Friend currentFriend_;

private:
    Neter neter_;
    UserService userService_;
    ChatService chatService_;
    Controller controller_;
};