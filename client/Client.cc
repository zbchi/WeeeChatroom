#include "Client.h"
#include "base.h"
#include <iostream>
#include <thread>
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Logger.h"

using namespace mylib;
Client::Client(const char *serverAddr) : serverAddr_(serverAddr),
                                         neter_(this, serverAddr),
                                         controller_(&neter_, this),
                                         chatService_(&neter_, this),
                                         userService_(&neter_, this),
                                         friendService_(&neter_, this),
                                         groupService_(&neter_, this),
                                         fileService_(&neter_, this)
{
    msgHandlerMap_[LOGIN_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->userService_.handleLoginAck(conn, js); };
    msgHandlerMap_[REG_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->userService_.handleRegAck(conn, js); };
    msgHandlerMap_[FIND_PASSWORD_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->userService_.handleFindAck(conn, js); };
    msgHandlerMap_[GET_FRIENDS] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleFriendsList(conn, js); };
    msgHandlerMap_[CHAT_MSG] = [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleMessage(conn, js); };
    msgHandlerMap_[ADD_FRIEND] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleFriendRequest(conn, js); };
    msgHandlerMap_[ADD_FRIEND_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleAddFriendAck(conn, js); };
    msgHandlerMap_[ADD_GROUP] = [this](const TcpConnectionPtr &conn, json &js)
    { this->groupService_.handleGroupRequest(conn, js); };
    msgHandlerMap_[ADD_GROUP_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->groupService_.handleGroupAddAck(conn, js); };
    msgHandlerMap_[GET_GROUPS] = [this](const TcpConnectionPtr &conn, json &js)
    { this->groupService_.handleGroupList(conn, js); };
    msgHandlerMap_[ADD_GROUP_REMOVE] = [this](const TcpConnectionPtr &conn, json &js)
    { this->groupService_.handleGroupRequestRemove(conn, js); };
    msgHandlerMap_[CHAT_GROUP_MSG] = [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleGroupMessage(conn, js); };
    msgHandlerMap_[GET_GROUPINFO] = [this](const TcpConnectionPtr &conn, json &js)
    { this->groupService_.handleGroupInfo(conn, js); };
    msgHandlerMap_[CHAT_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleMessageAck(conn, js); };
    msgHandlerMap_[CHAT_GROUP_MSG_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->chatService_.handleGroupMessageAck(conn, js); };
    msgHandlerMap_[GET_FILES] = [this](const TcpConnectionPtr &conn, json &js)
    { this->fileService_.handleFileList(conn, js); };
    msgHandlerMap_[BLOCK_FRIEND_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleBlockFriendAck(conn, js); };
    msgHandlerMap_[UNBLOCK_FRIEND_ACK] = [this](const TcpConnectionPtr &conn, json &js)
    { this->friendService_.handleUnblockFriendAck(conn, js); };
}

void Client::start()
{
    // 网络接收线程将消息放入消息队列
    neter_.start();
    controller_.mainLoop();
}

void Client::handleJson(const TcpConnectionPtr &conn, const std::string &jsonStr)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}
