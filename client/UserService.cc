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

void UserService::registerCode(std::string &email, std::string &password, std::string &nickname, int code)
{
    json regInfo;
    regInfo["msgid"] = REG_MSG_ACK;
    regInfo["email"] = email;
    regInfo["nickname"] = nickname;
    regInfo["password"] = password;
    regInfo["code"] = code;
    neter_->sendJson(regInfo);
}

void UserService::login(std::string &email, std::string &password)
{
    json loginInfo;
    loginInfo["msgid"] = LOGIN_MSG;
    loginInfo["email"] = email;
    loginInfo["password"] = password;

    neter_->sendJson(loginInfo);
}

void UserService::handleRegAck(const TcpConnectionPtr &conn, json &js)
{
    {
        std::unique_lock<std::mutex> lock(client_->controller_.reg_mtx_);
        client_->controller_.reg_errno_ = js["errno"];
        client_->controller_.regResultSet_ = true;
    }
    client_->controller_.regCv_.notify_one();
}
void UserService::handleLoginAck(const TcpConnectionPtr &conn, json &js)
{
    client_->user_email_ = js["email"];
    if (js["errno"] == 0)
        client_->user_id_ = js["user_id"];
    {
        std::unique_lock<std::mutex> lock(client_->controller_.login_mtx_);
        client_->controller_.login_errno_ = js["errno"];
        client_->controller_.loginResultSet_ = true;
    }
    client_->controller_.loginCv_.notify_one();
}

void UserService::addFriend(std::string &friendEmail)
{
    json addInfo;
    addInfo["msgid"] = ADD_FRIEND;
    addInfo["email"] = friendEmail;
    addInfo["user_id"] = client_->user_id_;
    neter_->sendJson(addInfo);
}

void UserService::getFriends()
{
    json getInfo;
    getInfo["msgid"] = GET_FRIENDS;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);
}

void UserService::handleFriendsList(const TcpConnectionPtr &conn, json &js)
{

    Friend f;
    for (const auto &afriend : js["friends"])
    {
        f.id_ = afriend["id"];
        f.nickname_ = afriend["nickname"];
        client_->firendList_.push_back(f);
    }
}