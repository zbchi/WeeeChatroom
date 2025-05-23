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

void UserService::addFriend(std::string &friendEmail)
{
    json addInfo;
    addInfo["msgid"] = ADD_FRIEND;
    addInfo["email"] = friendEmail;
    neter_->sendJson(addInfo);
}

void UserService::getFriends()
{
    json getInfo;
    getInfo["msgid"] = GET_FRIENDS;
    getInfo["user_id"] = client_->user_id_;
    neter_->sendJson(getInfo);
    json friendsList = client_->messageQueue_.pop();

    Friend f;
    for (const auto &afriend : friendsList["friends"])
    {
        f.id_ = afriend["id"];
        f.nickname_ = afriend["nickname"];
        client_->firendList_.push_back(f);
    }
}