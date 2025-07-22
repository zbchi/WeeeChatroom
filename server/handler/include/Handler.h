#pragma once
#include <nlohmann/json.hpp>
#include "MySQLConn.h"
#include "TcpConnection.h"
using json = nlohmann::json;
using namespace mylib;
class Handler
{
public:
    virtual void handle(const mylib::TcpConnectionPtr &conn, json &js, mylib::Timestamp time) = 0;
    virtual ~Handler() = default;
};