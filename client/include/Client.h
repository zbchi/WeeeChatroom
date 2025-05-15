#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <thread>

#include "Neter.h"
#include "UserService.h"
using namespace mylib;
using json = nlohmann::json;

class Client
{
public:
    Client();
    void start();

    using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    void handleMessage(const TcpConnectionPtr &conn, std::string &jsonStr, Timestamp time);

    void reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Neter neter_;
    UserService userService_;
};