#pragma once
#include <nlohmann/json.hpp>
#include "TcpConnection.h"
using json = nlohmann::json;
void sendJson(const mylib::TcpConnectionPtr &conn, json &js);
json makeResponse(int msgid, int errno_, std::string errmsg = "");
enum MsgType
{
    REG_MSG = 1,
    REG_MSG_ACK,
    LOGIN_MSG,
    LOGIN_MSG_ACK,
    GET_FRIENDS,
    CHAT_MSG,
    ADD_FRIEND,
    DEL_FRIEND,
    ADD_FRIEND_ACK,
    ADD_GROUP,
};

struct User
{
    std::string id;
    std::string nickname;
};