#pragma once
#include "Handler.h"

using namespace mylib;
class Service;
class FriendAddResponser;
class FriendDeleter;
class FriendLister : public Handler
{
    friend FriendAddResponser;
    friend FriendDeleter;

public:
    FriendLister(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void sendFriendList(std::string &user_id);

private:
    Result getFriendsId(std::string &user_id);
    Result getFriendsInfo(Result &friendsId);
    Service *service_;
};

class FriendAdder : public Handler
{
public:
    FriendAdder(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class FriendAddResponser : public Handler
{
public:
    FriendAddResponser(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class FriendDeleter : public Handler
{
public:
    FriendDeleter(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class FriendBlocker : public Handler
{
public:
    FriendBlocker(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class FriendUnblocker : public Handler
{
public:
    FriendUnblocker(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};