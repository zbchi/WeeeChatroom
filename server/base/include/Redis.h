#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <sw/redis++/redis++.h>
using namespace sw::redis;
extern std::shared_ptr<sw::redis::Redis> redis;
void initRedisConnPool();
/*
class Redis
{
public:
    redisContext *context_;
    Redis() : context_(nullptr) { connect(); };
    ~Redis() { disconnect(); };

    bool connect(const std::string &ip = "127.0.0.1", int port = 6379);
    bool disconnect();

    bool checkReply(redisReply *reply);
    bool expire(const std::string &key, int seconds);

    bool set(const std::string &key, const std::string &value);
    std::string get(const std::string &key);
    bool del(const std::string &key);

    bool hset(const std::string &key, const std::string &field, const std::string &value);
    std::string hget(const std::string &key, const std::string &field);
    bool hdel(const std::string &key, const std::string &field);
};*/
