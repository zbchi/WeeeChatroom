#pragma once
#include <nlohmann/json.hpp>
#include "TcpConnection.h"
#include "Logger.h"
#include <mutex>
#include <condition_variable>
using json = nlohmann::json;
using namespace mylib;
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
    DEL_FRIEND,
    ADD_FRIEND_ACK,
    CREATE_GROUP,
    ADD_GROUP,
    ADD_GROUP_ACK,
    GET_GROUPS,
    ADD_GROUP_REMOVE,
    GET_GROUPINFO,
    EXIT_GROUP,
    KICK_MEMBER,
    ADD_ADMIN,
    REMOVE_ADMIN,
};

class Waiter
{
public:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready = false;
    int result = -1;
    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]
                 { return ready; });
    }
    void notify(int res)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            result = res;
            ready = true;
        }
        cv_.notify_one();
    }

    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ready = false;
        result = -1;
    }

    int getResult()
    {
        int temp = result;
        reset();
        return temp;
    }
};