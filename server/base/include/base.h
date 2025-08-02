#pragma once
#include <nlohmann/json.hpp>
#include "TcpConnection.h"
#include "Redis.h"
using json = nlohmann::json;
using namespace mylib;

void sendJson(const mylib::TcpConnectionPtr &conn, const json &js);
void sendJson(const mylib::TcpConnectionPtr &conn, const std::string &jsonStr);
json makeResponse(int msgid, int errno_, std::string errmsg = "");

void initRedisConnPool();

enum MsgType
{
    REG_MSG = 1,
    REG_MSG_ACK,
    LOGIN_MSG,
    LOGIN_MSG_ACK,
    FIND_PASSWORD,
    FIND_PASSWORD_ACK,
    GET_FRIENDS,
    CHAT_MSG,
    CHAT_MSG_ACK,
    CHAT_GROUP_MSG,
    CHAT_GROUP_MSG_ACK,
    ADD_FRIEND,
    ADD_FRIEND_ACK,
    DEL_FRIEND,
    FRIEND_REQUEST,
    CREATE_GROUP,
    ADD_GROUP,
    ADD_GROUP_ACK,
    GROUP_REQUEST,
    GET_GROUPS,
    ADD_GROUP_REMOVE,
    GET_GROUPINFO,
    EXIT_GROUP,
    KICK_MEMBER,
    ADD_ADMIN,
    REMOVE_ADMIN,
    UPLOAD_FILE,
    GET_FILES,
    UPLOAD_FILE_ACK,
    DOWNLOAD_FILE,
    DOWNLOAD_FILE_ACK,
    BLOCK_FRIEND,
    BLOCK_FRIEND_ACK,
    UNBLOCK_FRIEND,
    UNBLOCK_FRIEND_ACK,
    DESTROY_ACCOUNT,
    GET_LOGS,
    GET_LOGS_ACK,
};