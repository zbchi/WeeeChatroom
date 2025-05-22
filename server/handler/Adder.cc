#include "Adder.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
void AdderFriend::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string friendEmail = js["email"];
    std::string user_id = conn->user_id;
    int errno_add = addFriend(friendEmail, user_id);
}

int AdderFriend::addFriend(std::string &friendEmail, std::string &user_id)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    char sql_c[128];
    snprintf(sql_c, sizeof(sql_c), "select * from users where email='%s'", friendEmail.c_str());
    std::string sql(sql_c);
    auto result=mysql->queryResult(sql);

    std::string friend_id;
    if(result.empty())
    {
        LOG_DEBUG("%s不存在好友",friendEmail.c_str());
        return 2;
    }
    else
    {
        friend_id=result[0]["id"];
    }

    
    


}