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
    void sendMessage(std::string &sender_id, std::string &reciver_id, std::string &content);
    void handleMessage(const TcpConnectionPtr &conn, json &js);
    std::string fixInvalidUtf8(const std::string &input);

    std::mutex chatLogs_mutex_;

private:
    Neter *neter_;
    Client *client_;
};

struct ChatMessage
{
    std::string sender_id;
    std::string content;
    std::string timestamp;
};

using ChatLog = std::vector<ChatMessage>;