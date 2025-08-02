#include "ChatService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"
#include "Timestamp.h"

#include "ui.h"
using namespace mylib;

int ChatService::sendMessage(std::string &content)
{
    {
        std::lock_guard<std::mutex> lock(client_->friendListMutex_);
        auto it = client_->friendList_.find(client_->currentFriend_.id_);
        if (it == client_->friendList_.end())
            return 1; // 已经不是好友
        else
        {
            if (it->second.is_blocked)
                return 2; // 被拉黑了
        }
    }
    json sendInfo;
    sendInfo["msgid"] = CHAT_MSG;
    sendInfo["sender_id"] = client_->user_id_;
    sendInfo["nickname"] = client_->nickname_;
    sendInfo["receiver_id"] = client_->currentFriend_.id_; // friend_id
    sendInfo["content"] = fixInvalidUtf8(content);

    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{client_->user_id_, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(chatLogs_mutex_);
        auto &logs = client_->chatLogs_[client_->currentFriend_.id_];
        logs.push_back(msg);
        if (logs.size() > 10)
            logs.pop_front();
    }
    neter_->sendJson(sendInfo);
    // chatMessageWaiter_.wait();
    // int result = chatMessageWaiter_.getResult();
    // if (result == 0)
    storeChatLog(client_->user_id_, client_->currentFriend_.id_, sendInfo);
    // client_->controller_.printALog(msg, false);
    /*if (state_ != State::FILE_FRIEND)
        client_->controller_.flushLogs();*/
    return 0;
}

void ChatService::handleMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string friend_id = js["sender_id"];
    std::string content = js["content"];
    std::string timestamp = js["timestamp"];
    std::string nickname = js["nickname"];

    // 将收到消息存入chatLogs_
    // ChatMessage msg{friend_id, content, timestamp, client_->user_id_};
    if (state_ == State::CHAT_FRIEND)
    {
        ChatMessage msg{friend_id, content, timestamp, client_->user_id_};
        {
            std::lock_guard<std::mutex> lock(chatLogs_mutex_);
            auto &logs = client_->chatLogs_[client_->currentFriend_.id_];
            logs.push_back(msg);
            if (logs.size() > 10)
                logs.pop_front();
        }
    }
    storeChatLog(client_->user_id_, friend_id, js);
    if ((state_ == State::CHAT_FRIEND) && friend_id == client_->currentFriend_.id_)
    {
        // loadInitChatLogs(client_->currentFriend_.id_, 20);
        client_->controller_.flushLogs();
        // client_->controller_.printALog(msg, false);
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(client_->isReadMapMutex_);
            client_->isReadMap_[friend_id] = true;
        }
        printTopBegin();
        std::cout << "好友[" << nickname << "]发来了一条新消息";
        printTopEnd();
    }
}

int ChatService::sendGroupMessage(std::string &content)
{
    {
        std::lock_guard<std::mutex> lock(client_->groupListMutex_);
        if (!client_->groupList_.count(client_->currentGroup_.group_id_))
            return 1; // 已经不再群里
    }
    json sendInfo;
    sendInfo["msgid"] = CHAT_GROUP_MSG;
    sendInfo["sender_id"] = client_->user_id_;
    sendInfo["group_id"] = client_->currentGroup_.group_id_;
    sendInfo["content"] = fixInvalidUtf8(content);
    std::string timestamp = Timestamp::now().toFormattedString();
    sendInfo["timestamp"] = timestamp;
    sendInfo["group_name"] = client_->currentGroup_.group_name;

    // 将发送单条消息存入chatLogs_
    ChatMessage msg{client_->user_id_, content, timestamp, client_->user_id_};
    {
        std::lock_guard<std::mutex> lock(groupChatLogs_mutex_);
        auto &logs = client_->groupChatLogs_[client_->currentGroup_.group_id_];
        logs.push_back(msg);
        if (logs.size() > 10)
            logs.pop_front();
    }
    neter_->sendJson(sendInfo);
    // chatGroupMessageWaiter_.wait();
    // int result = chatGroupMessageWaiter_.getResult();
    // if (result == 0)
    storeChatLog(client_->user_id_, client_->currentGroup_.group_id_, sendInfo, true);
    // client_->controller_.printALog(msg, true);
    /* if (state_ != State::FILE_GROUP)
         client_->controller_.flushGroupLogs();*/
    return 0;
}

void ChatService::handleGroupMessage(const TcpConnectionPtr &conn, json &js)
{
    std::string group_id = js["group_id"];
    std::string content = js["content"];
    std::string sender_id = js["sender_id"];
    std::string timestamp = js["timestamp"];
    std::string group_name = js["group_name"];
    // 将消息存入groupChatLogs_
    if (state_ == State::CHAT_GROUP)
    {
        ChatMessage msg{sender_id, content, timestamp, client_->user_id_};
        {
            std::lock_guard<std::mutex> lock(groupChatLogs_mutex_);
            auto &logs = client_->groupChatLogs_[group_id];
            logs.push_back(msg);
            if (logs.size() > 10)
                logs.pop_front();
        }
    }
    storeChatLog(client_->user_id_, group_id, js, true);
    if (state_ == State::CHAT_GROUP && group_id == client_->currentGroup_.group_id_)
    {
        client_->controller_.flushGroupLogs();
        // client_->controller_.printALog(msg, true);
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(client_->isReadGroupMapMutex_);
            client_->isReadGroupMap_[group_id] = true;
        }
        printTopBegin();
        std::cout << "群[" << group_name << "]发来了一条新消息" << std::endl;
        printTopEnd();
    }
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
    fs::path log_dir = "/tmp/chatclient/chat_logs";
    if (is_group)
        return (log_dir / ("g_" + user_id + "_" + friend_id + ".log")).string();
    else
        return (log_dir / (user_id + "_" + friend_id + ".log")).string();
}

void ChatService::storeChatLog(std::string &user_id, std::string &peer_id, json &js, bool is_group)
{
    std::string path = getLogPath(user_id, peer_id, is_group);
    fs::create_directories(fs::path(path).parent_path());
    std::ofstream ofs(path, std::ios::app);
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

void ChatService::loadInitChatLogs(std::string &peer_id, ssize_t count, bool is_group)
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

void ChatService::loadMoreChatLogs(std::string &peer_id, ssize_t count, ssize_t offset, bool is_group)
{
    ChatLogs logs = loadChatLogs(client_->user_id_, peer_id, count, offset, is_group);
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

void ChatService::getChatLogs(bool is_group)
{
    std::string peer_id = is_group ? client_->currentGroup_.group_id_ : client_->currentFriend_.id_;
    std::string log_path = getLogPath(client_->user_id_, peer_id, is_group);
    if (fs::exists(log_path))
        return;
    json getInfo;
    getInfo["msgid"] = GET_LOGS;
    getInfo["is_group"] = is_group;
    getInfo["peer_id"] = peer_id;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);
    // getLogsWaiter_.wait();
}