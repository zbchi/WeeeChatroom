#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <thread>

#include "Neter.h"
#include "UserService.h"
#include "MessageQueue.h"
#include "Handler.h"
#include "Controller.h"
using namespace mylib;
using json = nlohmann::json;

class Client
{
    friend class Controller;

public:
    Client();
    void start();

    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;
    MessageQueue messageQueue_;
    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr);

private:
    Neter neter_;
    UserService userService_;
    Controller controller_;
};