#include "UserService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"
#include "Client.h"
void UserService::regiSter(std::string &email, std::string &password, std::string &nickname)
{
    json regInfo;
    regInfo["msgid"] = REG_MSG;
    regInfo["email"] = email;
    regInfo["nickname"] = nickname;
    regInfo["password"] = password;

    neter_->sendJson(regInfo);
}

int UserService::registerCode(std::string &email, std::string &password, std::string &nickname, int code)
{
    json regInfo;
    regInfo["msgid"] = REG_MSG_ACK;
    regInfo["email"] = email;
    regInfo["nickname"] = nickname;
    regInfo["password"] = password;
    regInfo["code"] = code;
    neter_->sendJson(regInfo);

    // 阻塞等待回应
    regWaiter_.wait();
    return regWaiter_.getResult();
}

int UserService::login(std::string &email, std::string &password)
{
    json loginInfo;
    loginInfo["msgid"] = LOGIN_MSG;
    loginInfo["email"] = email;
    loginInfo["password"] = password;
    neter_->sendJson(loginInfo);

    // 阻塞等待回应
    loginWaiter_.wait();
    return loginWaiter_.getResult();
}

void UserService::handleRegAck(const TcpConnectionPtr &conn, json &js)
{
    regWaiter_.notify(js["errno"]);
}

void UserService::handleLoginAck(const TcpConnectionPtr &conn, json &js)
{
    client_->user_email_ = js["email"];
    if (js["errno"] == 0)
        client_->user_id_ = js["user_id"];
    loginWaiter_.notify(js["errno"]);
}

void UserService::findPassword(std::string &email)
{
    json findInfo;
    findInfo["msgid"] = FIND_PASSWORD;
    findInfo["email"] = email;
    neter_->sendJson(findInfo);
}

int UserService::findPasswordCode(std::string &email, std::string &password, int code)
{
    json findInfo;
    findInfo["msgid"] = FIND_PASSWORD_ACK;
    findInfo["email"] = email;
    findInfo["password"] = password;
    findInfo["code"] = code;
    neter_->sendJson(findInfo);

    // 阻塞等待回应
    findWaiter_.wait();
    return findWaiter_.getResult();
}

void UserService::handleFindAck(const TcpConnectionPtr &conn, json &js)
{
    findWaiter_.notify(js["errno"]);
}

void UserService::destroyAccount()
{
    json desInfo;
    desInfo["msgid"]=DESTROY_ACCOUNT;
    desInfo["user_id"]=client_->user_id_;
    neter_->sendJson(desInfo);
}