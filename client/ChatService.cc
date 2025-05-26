#include "ChatService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"
#include "Timestamp.h"
using namespace mylib;
void ChatService::sendMessage(std::string &sender_id, std::string &reciver_id, std::string &content)
{
    json sendInfo;
    sendInfo["msgid"] = CHAT_MSG;
    sendInfo["sender_id"] = sender_id;
    sendInfo["reciver_id"] = reciver_id; // friend_id
    sendInfo["content"] = content;
    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{sender_id, content, timestamp};
    client_->chatLogs_[reciver_id].push_back(msg);
    neter_->sendJson(sendInfo);
}

void ChatService::handleMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string friend_id = js["sender_id"];
    std::string content = js["content"];
    std::string timestamp = js["timestamp"];

    // 将收到消息存入chatLogs_
    ChatMessage msg{friend_id, content, timestamp};
    client_->chatLogs_[friend_id].push_back(msg);
    std::cout << "收到了一条消息" << std::endl;
}