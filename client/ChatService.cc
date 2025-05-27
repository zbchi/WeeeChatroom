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

void ChatService::sendMessage(std::string &sender_id, std::string &reciver_id, std::string &content)
{
    json sendInfo;
    sendInfo["msgid"] = CHAT_MSG;
    sendInfo["sender_id"] = sender_id;
    sendInfo["reciver_id"] = reciver_id; // friend_id
    sendInfo["content"] = fixInvalidUtf8(content);
    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{sender_id, content, timestamp};
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        client_->chatLogs_[reciver_id].push_back(msg);
    }
    neter_->sendJson(sendInfo);
}

void ChatService::handleMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string friend_id = js["sender_id"];
    std::string content = js["content"];
    std::string timestamp = js["timestamp"];

    // 将收到消息存入chatLogs_
    ChatMessage msg{friend_id, content, timestamp};
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        client_->chatLogs_[friend_id].push_back(msg);
    }
    std::cout << "收到了一条消息" << std::endl;
}