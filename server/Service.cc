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

Service::Service() : threadPool_(32),
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
    handlers_[FRIEND_REQUEST] = std::make_shared<FriendAddResponser>(this);
    handlers_[CREATE_GROUP] = std::make_shared<GroupCreater>(this);
    handlers_[ADD_GROUP] = std::make_shared<GroupAdder>(this);
    handlers_[GET_GROUPS] = std::make_shared<GroupLister>(this);
    handlers_[GROUP_REQUEST] = std::make_shared<GroupAddResponser>(this);
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
    handlers_[BLOCK_FRIEND] = std::make_shared<FriendBlocker>(this);
    handlers_[DESTROY_ACCOUNT] = std::make_shared<AccountKiller>(this);
    handlers_[UNBLOCK_FRIEND] = std::make_shared<FriendUnblocker>(this);

    // 设置连接回调
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                  { this->onConnection(conn); });
    // 设置消息回调
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)

                               { this->onMessage(conn, buf, time); });
}

void Service::start()
{
    this->initCache();
    std::thread t([this]()
                  { FtpServer ftpServer_;
                    ftpServer_.start(); }); // 启动文件传输服务器
    loop_.runEvery(20.0, [this]()
                   { this->heartBeatCheck(); }); // 定时检测心跳
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
        // 创建心跳检测上下文
        Timestamp now = Timestamp::now();
        auto ctx = std::make_shared<HeartBeatContext>(now);
        conn->setContext(ctx);
    }
    else
    {
        // 断开连接讲用户移出在线用户表
        std::lock_guard<std::mutex> lock(onlienUsersMutex_);
        for (auto it = onlineUsers_.begin(); it != onlineUsers_.end(); it++)
        {
            if (it->second == conn)
            {
                onlineUsers_.erase(it);
                break;
            }
        }
    }
}

TcpConnectionPtr Service::getConnectionPtr(std::string user_id)
{ // 根据user ID 获取 connection指针
    std::lock_guard<std::mutex> lock(onlienUsersMutex_);
    auto it = onlineUsers_.find(user_id);
    if (it != onlineUsers_.end())
        return it->second;
    else
        return nullptr;
}

std::string Service::getUserid(const TcpConnectionPtr &conn)
{ // 根据connection指针 获取 user ID
    std::lock_guard<std::mutex> lock(onlienUsersMutex_);
    for (const auto &pair : onlineUsers_)
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
        if (jsonStr == "null") // 将心跳检测处理优先级提到线程池前，防止巨量json请求淹没心跳
            this->handleHeartBeat(conn, time);
        // 将json字符串分配给线程池
        else
            threadPool_.add_task([conn, jsonStr, time, this]()
                                 { handleMessage(conn, jsonStr, time); });
    }
}

void Service::setNumThreads(int numThreads)
{
    server_.setThreadNum(numThreads);
}

void Service::heartBeatCheck()
{
    Timestamp now = Timestamp::now();
    for (const auto pair : server_.connections_)
    {
        auto conn = pair.second;
        auto ctx = std::any_cast<std::shared_ptr<HeartBeatContext>>(conn->getContext());
        if (now.microSecondsSinceEpoch() - ctx->lastCheckTime.microSecondsSinceEpoch() > 30 * 1000 * 1000)
        {
            LOG_WARN("%s:连接超时强制关闭", conn->name().c_str());
            conn->forceClose();
        }
        else
            LOG_INFO("%s:heatbeat.", conn->name().c_str());
    }
}

void Service::handleHeartBeat(const TcpConnectionPtr &conn, Timestamp time)
{
    auto ctx = std::any_cast<std::shared_ptr<HeartBeatContext>>(conn->getContext());
    ctx->lastCheckTime = time;
}

void Service::initCache()
{
    threadPool_.add_task([]() { // 将好友关系放入redis缓存
        auto mysql = MySQLConnPool::instance().getConnection();
        Result result = mysql->select("friends");
        for (const auto &row : result)
        {
            std::string user_id = row.at("user_id");
            std::string friend_id = row.at("friend_id");
            redis->sadd("friends:" + user_id, friend_id); // 加入当前用户拥有的好友
            /*   friends:user_id  {好友id1,好友id2,好友id3,好友id4}   */
        };
        LOG_INFO("初始化好友关系缓存成功");
    });

    threadPool_.add_task([]() { // 将群成员放入redis缓存
        auto mysql = MySQLConnPool::instance().getConnection();
        Result result = mysql->select("group_members");
        for (const auto &row : result)
        {
            std::string group_id = row.at("group_id");
            std::string user_id = row.at("user_id");
            redis->sadd("group:" + group_id, user_id); // 加入当前群聊拥有的群成员
            /*   group:group_id    {成员id1,i成员id2,成员id3}*/
        }
        LOG_INFO("初始化群成员关系缓存成功");
    });
}