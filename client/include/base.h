#pragma once
#include <nlohmann/json.hpp>
#include "TcpConnection.h"
using json = nlohmann::json;
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
    CREATE_GROUP,
    ADD_GROUP,
};