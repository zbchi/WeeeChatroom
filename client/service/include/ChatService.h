#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <fstream>

#include "Neter.h"
#include "base.h"
class Neter;
class Client;
using namespace mylib;
namespace fs = std::filesystem;

struct ChatMessage
{
    std::string sender_id;
    std::string content;
    std::string timestamp;
    std::string user_id_;
};
using ChatLogs = std::vector<ChatMessage>;

class ChatService
{
public:
    ChatService(Neter *neter, Client *client) : neter_(neter), client_(client) {}
    int sendMessage(std::string &content);
    int sendGroupMessage(std::string &content);
    void handleMessage(const TcpConnectionPtr &conn, json &js);
    void handleGroupMessage(const TcpConnectionPtr &conn, json &js);
    void handleMessageAck(const TcpConnectionPtr &conn, json &js);
    void handleGroupMessageAck(const TcpConnectionPtr &conn, json &js);

    void loadInitChatLogs(std::string &peer_id, ssize_t count = 20, bool is_group = false);
    void loadMoreChatLogs(std::string &peer_id, ssize_t count, ssize_t offset, bool is_group = false);
    std::mutex chatLogs_mutex_;
    std::mutex groupChatLogs_mutex_;

    Waiter chatMessageWaiter_;
    Waiter chatGroupMessageWaiter_;

private:
    std::string getLogPath(std::string &user_id,
                           std::string &friend_id,
                           bool is_group = false);
    void storeChatLog(std::string &user_id,
                      std::string &peer_id,
                      json &js,
                      bool is_group = false);
    ChatLogs loadChatLogs(std::string &user_id,
                          std::string &peer_id,
                          size_t count,
                          size_t offset = 0,
                          bool is_group = false);
    std::string fixInvalidUtf8(const std::string &input);
    Neter *neter_;
    Client *client_;
};
