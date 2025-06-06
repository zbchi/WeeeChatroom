#include "ChatService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"
#include "Timestamp.h"
using namespace mylib;
std::string ChatService::fixInvalidUtf8(const std::string &input)
{
    std::string result;
    size_t i = 0;
    while (i < input.size())
    {
        unsigned char c = input[i];
        size_t len = 0;
        if (c <= 0x7F)
            len = 1; // ASCII
        else if ((c & 0xE0) == 0xC0)
            len = 2; // 2-byte
        else if ((c & 0xF0) == 0xE0)
            len = 3; // 3-byte
        else if ((c & 0xF8) == 0xF0)
            len = 4; // 4-byte
        else
        {
            i++;
            continue;
        } // invalid leading byte

        if (i + len <= input.size())
        {
            result.append(input.substr(i, len));
        }
        i += len;
    }
    return result;
}

void ChatService::sendMessage(std::string &content)
{
    json sendInfo;
    sendInfo["msgid"] = CHAT_MSG;
    sendInfo["sender_id"] = client_->user_id_;
    sendInfo["receiver_id"] = client_->currentFriend_.id_; // friend_id
    sendInfo["content"] = fixInvalidUtf8(content);
    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{client_->user_id_, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        client_->chatLogs_[client_->currentFriend_.id_].push_back(msg);
    }
    neter_->sendJson(sendInfo);
}

void ChatService::handleMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string friend_id = js["sender_id"];
    std::string content = js["content"];
    std::string timestamp = js["timestamp"];

    // 将收到消息存入chatLogs_
    ChatMessage msg{friend_id, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        client_->chatLogs_[friend_id].push_back(msg);
    }
    if (state_ == State::CHAT_FRIEND && friend_id == client_->currentFriend_.id_)
        client_->controller_.flushLogs();
}

void ChatService::sendGroupMessage(std::string &content)
{
    json sendInfo;
    sendInfo["msgid"] = CHAT_GROUP_MSG;
    sendInfo["sender_id"] = client_->user_id_;
    sendInfo["group_id"] = client_->currentGroup_.group_id_;
    sendInfo["content"] = fixInvalidUtf8(content);
    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{client_->user_id_, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(groupChatLogs_mutex_);
        client_->groupChatLogs_[client_->currentGroup_.group_id_].push_back(msg);
    }
    neter_->sendJson(sendInfo);
}

void ChatService::handleGroupMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string group_id = js["group_id"];
    std::string content = js["content"];
    std::string sender_id = js["sender_id"];
    std::string timestamp = js["timestamp"];

    // 将消息存入groupChatLogs_
    ChatMessage msg{sender_id, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(groupChatLogs_mutex_);
        client_->groupChatLogs_[group_id].push_back(msg);
    }
    if (state_ == State::CHAT_GROUP && group_id == client_->currentGroup_.group_id_)
        client_->controller_.flushGroupLogs();
}