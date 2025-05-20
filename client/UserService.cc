#include "UserService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"

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