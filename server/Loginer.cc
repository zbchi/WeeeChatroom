#include "Loginer.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
#include <curl/curl.h>
void Loginer::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string email = js["email"].get<std::string>();
    std::string password = js["password"].get<std::string>();

    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["email"] = email;
    int errno_verify = verifyAccount(email, password);
    if (errno_verify == 0)
    {
        response["errno"] = 0;
        response["errmsg"] = "";
    }
    else if (errno_verify == 2)
    {
        response["errno"] = 2;
        response["errmsg"] = "该邮箱未注册";
    }
    else if (errno_verify == 1)
    {
        response["errno"] = 1;
        response["errmsg"] = "密码错误";
    }
    sendJson(conn, response);
}

int Loginer::verifyAccount(std::string &email, std::string &password)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    char sql_c[128];
    snprintf(sql_c, sizeof(sql_c), "select * from users where email='%s'", email.c_str());
    std::string sql(sql_c);
    auto result = mysql->queryResult(sql);
    if (result.empty())
    {
        LOG_DEBUG("%s未注册", email.c_str());
        return 2;
    }

    if (result[0]["password"] == password)
    {
        LOG_DEBUG("密码正确");
        return 0;
    }
    else
    {
        LOG_DEBUG("密码错误");
        return 1;
    }
}