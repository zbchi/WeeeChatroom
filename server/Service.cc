#include <string>
#include <iostream>
#include <ctime>

#include "Service.h"
#include "Register.h"
#include "Login.h"
#include "Friend.h"
#include "Chat.h"
#include "Group.h"
#include "File.h"

#include "MySQLConn.h"
#include "base.h"
#include <arpa/inet.h>

Service::Service() : threadPool_(16),
                     listenAddr_(8000),
                     server_(&loop_, listenAddr_)
{
    handlers_[REG_MSG] = std::make_shared<Register>(this);
    handlers_[REG_MSG_ACK] = std::make_shared<RegisterAcker>(this);
    handlers_[LOGIN_MSG] = std::make_shared<Loginer>(this);
    handlers_[GET_FRIENDS] = std::make_shared<FriendLister>(this);
    handlers_[CHAT_MSG] = std::make_shared<Chatter>(this);
    handlers_[ADD_FRIEND] = std::make_shared<FriendAdder>(this);
    handlers_[DEL_FRIEND] = std::make_shared<FriendDeleter>(this);
    handlers_[ADD_FRIEND_ACK] = std::make_shared<FriendAddAcker>(this);
    handlers_[CREATE_GROUP] = std::make_shared<GroupCreater>(this);
    handlers_[ADD_GROUP] = std::make_shared<GroupAdder>(this);
    handlers_[GET_GROUPS] = std::make_shared<GroupLister>(this);
    handlers_[ADD_GROUP_ACK] = std::make_shared<GroupAddAcker>(this);
    handlers_[CHAT_GROUP_MSG] = std::make_shared<GroupChatter>(this);
    handlers_[GET_GROUPINFO] = std::make_shared<GroupInfoSender>(this);
    handlers_[EXIT_GROUP] = std::make_shared<GroupExiter>(this);
    handlers_[KICK_MEMBER] = std::make_shared<MemberKicker>(this);
    handlers_[ADD_ADMIN] = std::make_shared<AdminAdder>(this);
    handlers_[REMOVE_ADMIN] = std::make_shared<AdminRemover>(this);
    handlers_[FIND_PASSWORD] = std::make_shared<PasswordFinder>(this);
    handlers_[FIND_PASSWORD_ACK] = std::make_shared<PasswordFindAcker>(this);
    handlers_[UPLOAD_FILE] = std::make_shared<FileUploader>(this);
    handlers_[GET_FILES] = std::make_shared<FileLister>(this);

    // 设置连接回调
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                  { this->onConnection(conn); });
    // 设置消息回调
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)

                               { this->onMessage(conn, buf, time); });
}

void Service::start()
{
    std::thread t([this]()
                  {
    FtpServer ftpServer_;
    ftpServer_.start(); });
    server_.start();
    loop_.loop();
}

void Service::handleMessage(const mylib::TcpConnectionPtr &conn,
                            const std::string &jsonStr, mylib::Timestamp time)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = handlers_.find(msgid);
    if (it != handlers_.end())
        it->second->handle(conn, data, time);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}

void Service::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
    }
    else
    { // 断开连接讲用户移出在线用户表
        std::lock_guard<std::mutex> lock(onlienUsersMutex_);
        for (auto it = onlienUsers_.begin(); it != onlienUsers_.end(); it++)
        {
            if (it->second == conn)
            {
                onlienUsers_.erase(it);
                break;
            }
        }
    }
}

TcpConnectionPtr Service::getConnectionPtr(std::string user_id)
{ // 根据user ID 获取 connection指针
    std::lock_guard<std::mutex> lock(onlienUsersMutex_);
    auto it = onlienUsers_.find(user_id);
    if (it != onlienUsers_.end())
        return it->second;
    else
        return nullptr;
}

std::string Service::getUserid(const TcpConnectionPtr &conn)
{ // 根据connection指针 获取 user ID
    std::lock_guard<std::mutex> lock(onlienUsersMutex_);
    for (const auto &pair : onlienUsers_)
    {
        if (pair.second == conn)
            return pair.first;
    }
    return "";
}

void Service::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    while (buf->readableBytes() >= 4)
    { // 消息回调时处理TCP粘包
        const void *data = buf->peek();
        int lenNetOrder;
        memcpy(&lenNetOrder, data, sizeof(lenNetOrder));
        int len = ntohl(lenNetOrder);
        if (buf->readableBytes() < 4 + len)
            break;
        buf->retrieve(4);

        std::string jsonStr(buf->peek(), len);
        buf->retrieve(len);
        std::cout << jsonStr << std::endl;
        // 将json字符串分配给线程池
        threadPool_.add_task([conn, jsonStr, time, this]()
                             { handleMessage(conn, jsonStr, time); });
    }
}

void Service::setNumThreads(int numThreads)
{
    server_.setThreadNum(numThreads);
}