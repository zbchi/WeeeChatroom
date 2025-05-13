#include <string>
#include <iostream>
#include <ctime>

#include "Service.h"
#include "login.h"
#include "base.h"
#include <arpa/inet.h>
Service::Service()
{
    msgHandlerMap_[REG_MSG] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
    { this->reg(conn, js, time); };

    msgHandlerMap_[REG_MSG_ACK] =
        [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
    { this->reg_ack(conn, js, time); };
}

void Service::handleMessage(const TcpConnectionPtr &conn,
                            std::string &jsonStr, Timestamp time)
{
    json data = json::parse(jsonStr);
    int msgid = data["msgid"].get<int>();

    auto it = msgHandlerMap_.find(msgid);
    if (it != msgHandlerMap_.end())
        it->second(conn, data, time);
    else
        LOG_ERROR("无法解析此命令 %d", msgid);
}

void Service::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int code = gVerificationCode();
    std::string email = js["email"].get<std::string>();
    bool isStore = storeCode(email, code);
    bool isSend = sendCode(email, code);
}

void Service::reg_ack(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int inputCode = js["code"].get<int>();
    std::string email = js["email"].get<std::string>();
    std::string password = js["password"].get<std::string>();
    std::string nickname = js["nickname"].get<std::string>();

    json response;
    response["msgid"] = REG_MSG_ACK;

    int errno_verify = verifyCode(email, inputCode);
    if (errno_verify == 0)
    {
        if (inputAccount(email, password, nickname, response))
            response["errno"] = 0;
        else
        {
            response["errno"] = -1;
            response["errmsg"] = "注册失败";
        }
    }
    else if (errno_verify == 2)
    {
        response["errno"] = 2;
        response["errmsg"] = "该邮箱已注册";
    }
    else if (errno_verify == 1)
    {
        response["errno"] = 1;
        response["errmsg"] = "验证码错误";
    }
    sendJson(conn, response);
}