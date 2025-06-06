#pragma once
#include <string>
#include <vector>
#include <mutex>

#include "Neter.h"

class Neter;
class Client;
using namespace mylib;
class ChatService
{
public:
    ChatService(Neter *neter, Client *client) : neter_(neter), client_(client) {}
    void sendMessage(std::string &content);
    void handleMessage(const TcpConnectionPtr &conn, json &js);
    void sendGroupMessage(std::string &content);
    void handleGroupMessage(const TcpConnectionPtr &conn, json &js);

    std::string fixInvalidUtf8(const std::string &input);

    std::mutex chatLogs_mutex_;
    std::mutex groupChatLogs_mutex_;

private:
    Neter *neter_;
    Client *client_;
};

struct ChatMessage
{
    std::string sender_id;
    std::string content;
    std::string timestamp;
    std::string user_id_;
};

using ChatLog = std::vector<ChatMessage>;