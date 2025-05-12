#include "redis.h"
#include "Logger.h"

bool Redis::checkReply(redisReply *reply)
{
    if (reply == nullptr)
    {
        LOG_ERROR("Redis命令执行失败:回复为空");
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        LOG_ERROR("Redis命令执行错误:%s", reply->str);
        freeReplyObject(reply);
        return false;
    }
    return true;
}

bool Redis::connect(const std::string &ip, int port)
{
    if (context_ != nullptr)
        disconnect();

    context_ = redisConnect(ip.c_str(), port);
    if (context_ == nullptr || context_->err)
    {
        if (context_)
        {
            LOG_ERROR("Redis连接失败:%s", context_->errstr);
            redisFree(context_);
        }
        else
            LOG_ERROR("Redis连接失败:无法分配内存");

        context_ = nullptr;
        return false;
    }
    LOG_TRACE("Redis连接成功");
    return true;
}

bool Redis::disconnect()
{
    if (context_ != nullptr)
    {
        redisFree(context_);
        context_ = nullptr;
    }
    return true;
}

bool Redis::set(const std::string &key, const std::string &value)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "SET %s %s", key.c_str(), value.c_str());
    if (!checkReply(reply))
        return false;

    bool success = (reply->type == REDIS_REPLY_STATUS &&
                    strcasecmp("OK", reply->str));
    freeReplyObject(reply);
    return success;
}

std::string Redis::get(const std::string &key)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "GET %s", key.c_str());
    if (!checkReply(reply))
        return "";

    std::string value;
    if (reply->type == REDIS_REPLY_STRING)
        value = reply->str;
    freeReplyObject(reply);
    return value;
}

bool Redis::del(const std::string &key)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "DEL %s", key.c_str());
    if (!checkReply(reply))
        return false;
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

bool Redis::hset(const std::string &key, const std::string &field, const std::string &valule)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "HSET %s %s %s",
                     key.c_str(), field.c_str(), valule.c_str());
    if (!checkReply(reply))
        return false;
    bool success = (reply->type == REDIS_REPLY_INTEGER);
    freeReplyObject(reply);
    return success;
}

std::string Redis::hget(const std::string &key, const std::string &field)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "HGET %s %s", key.c_str(), field.c_str());
    if (!checkReply(reply))
        return "";

    std::string value;
    if (reply->type == REDIS_REPLY_STRING)
        value = reply->str;
    freeReplyObject(reply);
    return value;
}

bool Redis::hdel(const std::string &key, const std::string &field)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "HDEL %s %s", key.c_str(), field.c_str());
    if (!checkReply(reply))
        return false;

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

bool Redis::expire(const std::string &key, int seconds)
{
    redisReply *reply = (redisReply *)
        redisCommand(context_, "EXPIRE %s %d", key.c_str(), seconds);
    if (!checkReply(reply))
        return false;

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}