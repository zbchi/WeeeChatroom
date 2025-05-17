#pragma once
#include <nlohmann/json.hpp>
#include "Logger.h"
#include "TcpConnection.h"
using json = nlohmann::json;
using namespace mylib;
class Handler
{
public:
    virtual void handle(const mylib::TcpConnectionPtr &conn, json &js) = 0;
    virtual ~Handler() = default;
};