#include <cstdlib>
#include <ctime>
#include <hiredis/hiredis.h>
#include "Logger.h"
#include "login.h"
int gVerificationCode()
{
    std::srand(std::time(nullptr));
    int verificationCode = 100000 + (std::rand() % 900000);
    return verificationCode;
}

bool storeCode(std::string &email, int code, int expireTime = 120)
{
    redisContext *redis = redisConnect("127.0.0.1", 6379);
    if (redis == nullptr || redis->err)
        LOG_ERROR("无法连接Reids: %p", redis ? redis->errstr : "nullptr");

    std::string command = "SET " + email + " " + std::to_string(code) +
                          " EX " + std::to_string(expireTime);
    redisReply *reply = (redisReply *)redisCommand(redis, command.c_str());
    bool success = (reply != nullptr && reply->type != REDIS_REPLY_ERROR);
    if (!success)
        LOG_ERROR("Redis存储失败: %p", reply ? reply->str : "nullptr");

    freeReplyObject(reply);
    redisFree(redis);
    return success;
}

bool sendCode(std::string &email,int code)
{
    
}