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
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        else
        {
            i++;
            continue;
        }

        if (i + len <= input.size())
            result.append(input.substr(i, len));
        i += len;
    }
    return result;
}

int ChatService::sendMessage(std::string &content)
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

    chatMessageWaiter_.wait();
    int result = chatMessageWaiter_.getResult();
    if (result == 0)
        storeChatLog(client_->user_id_, client_->currentFriend_.id_, sendInfo);
    return result;
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
    storeChatLog(client_->user_id_, friend_id, js);
    if (state_ == State::CHAT_FRIEND && friend_id == client_->currentFriend_.id_)
        client_->controller_.flushLogs();
}

int ChatService::sendGroupMessage(std::string &content)
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
    chatGroupMessageWaiter_.wait();
    int result = chatGroupMessageWaiter_.getResult();
    if (result == 0)
        storeChatLog(client_->user_id_, client_->currentGroup_.group_id_, sendInfo, true);
    return result;
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
    storeChatLog(client_->user_id_, group_id, js, true);
    if (state_ == State::CHAT_GROUP && group_id == client_->currentGroup_.group_id_)
        client_->controller_.flushGroupLogs();
}

void ChatService::handleMessageAck(const TcpConnectionPtr &conn, json &js)
{
    int chat_errno = js["errno"];
    chatMessageWaiter_.notify(chat_errno);
}

void ChatService::handleGroupMessageAck(const TcpConnectionPtr &conn, json &js)
{
    int chat_errno = js["errno"];
    chatGroupMessageWaiter_.notify(chat_errno);
}

std::string ChatService::getLogPath(std::string &user_id, std::string &friend_id, bool is_group)
{
    fs::path exe_path = fs::canonical("/proc/self/exe");
    fs::path exe_dir = exe_path.parent_path();
    fs::path log_dir = exe_dir / "chat_logs";
    if (is_group)
        return (log_dir / ("g_" + user_id + "_" + friend_id + ".log")).string();
    else
        return (log_dir / (user_id + "_" + friend_id + ".log")).string();
}

void ChatService::storeChatLog(std::string &user_id, std::string &peer_id, json &js, bool is_group)
{
    std::string path = getLogPath(user_id, peer_id, is_group);
    std::ofstream ofs(path, std::ios::app);
    fs::create_directories(fs::path(path).parent_path());
    if (ofs.is_open())
        ofs << js.dump() << "\n";
}

ChatLogs ChatService::loadChatLogs(std::string &user_id,
                                   std::string &peer_id,
                                   size_t count,
                                   size_t offset,
                                   bool is_group)
{
    std::ifstream ifs(getLogPath(user_id, peer_id, is_group));
    if (!ifs.is_open())
        return {};
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifs, line))
        lines.push_back(line);

    ChatLogs logs;
    size_t total = lines.size();
    if (offset >= total)
        return {};
    size_t start = (total > offset + count) ? total - offset - count : 0;
    size_t end = total - offset;
    for (size_t i = start; i < end; i++)
    {
        json js = json::parse(lines[i]);
        std::string content = js["content"];
        std::string timestamp = js["timestamp"];
        std::string sender_id = js["sender_id"];
        logs.push_back(ChatMessage{sender_id, content, timestamp, user_id});
    }
    return logs;
}

void ChatService::loadInitChatLogs(std::string &peer_id, bool is_group)
{
    ChatLogs logs = loadChatLogs(client_->user_id_, peer_id, 20, 0, is_group);
    if (is_group)
    {
        std::lock_guard<std::mutex> lock(groupChatLogs_mutex_);
        client_->groupChatLogs_[peer_id] = logs;
    }
    else
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        client_->chatLogs_[peer_id] = logs;
    }
}
