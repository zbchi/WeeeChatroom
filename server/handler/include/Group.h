#pragma once
#include "Handler.h"

using namespace mylib;
class Service;

class GroupLister : public Handler
{
public:
    GroupLister(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void sendGroupList(std::string &user_id);

private:
    Result getGroupsId(std::string &user_id);
    Result getGroupsInfo(Result &groupsId);
    Service *service_;
};
class GroupCreater : public Handler
{
public:
    GroupCreater(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class GroupAdder : public Handler
{
public:
    GroupAdder(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class GroupAddResponser : public Handler
{
public:
    GroupAddResponser(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class GroupInfoSender : public Handler
{
public:
    GroupInfoSender(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class GroupExiter : public Handler
{
public:
    GroupExiter(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class MemberKicker : public Handler
{
public:
    MemberKicker(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class AdminAdder : public Handler
{
public:
    AdminAdder(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class AdminRemover : public Handler
{
public:
    AdminRemover(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};