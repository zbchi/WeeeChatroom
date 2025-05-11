#include <cstdlib>
#include <ctime>

#include <curl/curl.h>
#include "Logger.h"
#include "login.h"
#include <curl/easy.h>
int gVerificationCode()
{
    std::srand(std::time(nullptr));
    int verificationCode = 100000 + (std::rand() % 900000);
    return verificationCode;
}

bool storeCode(redisContext *redis, std::string &email, int code, int expireTime)
{

    std::string key = "verify_email:" + email;
    redisReply *reply =
        (redisReply *)redisCommand(redis, "HSET %s code %s EX %s",
                                   key.c_str(), std::to_string(code),
                                   std::to_string(expireTime));
    bool success = (reply != nullptr && reply->type != REDIS_REPLY_ERROR);
    if (!success)
        LOG_ERROR("Redis存储失败: %p", reply ? reply->str : "nullptr");

    freeReplyObject(reply);
    return success;
}

bool verifyCode(redisContext *redis, std::string &email, int inputCode)
{
    std::string key = "verify_email:" + email;
    redisReply *reply = (redisReply *)redisCommand(redis, "HMGET %s code", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements != 1)
    {
        if (reply)
            freeReplyObject(reply);
        return false;
    }

    std::string real_code = reply->element[0]->str ? reply->element[0]->str : "";
    freeReplyObject(reply);
    if (real_code != std::to_string(inputCode))
        return false;

    redisCommand(redis, "DEL %s", key.c_str());
    return true;
}

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    std::string *payload_text = static_cast<std::string *>(userp);
    static size_t pos = 0;

    if (pos >= payload_text->length())
    {
        pos = 0;
        return 0;
    }

    size_t available = payload_text->length() - pos;
    size_t room = size * nmemb;
    size_t will_send = (available < room) ? available : room;

    memcpy(ptr, payload_text->c_str() + pos, will_send);
    pos += will_send;

    return will_send;
}

bool sendCode(std::string &email, int code)
{
    CURLcode res = CURLE_OK;
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // 打印通信日志

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.qq.com:465");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "1662308219@qq.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "xrnkziqguktncfbe");
        curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");

        const std::string from = "From: <1662308219@qq.com>\r\n";
        const std::string to = "To: <" + email + ">\r\n";
        const std::string type = "Content-Type: text/html;\r\n";
        const std::string subject = "Subject: Chatroom Verification Code\r\n";
        const std::string body = "使用以下验证码以完成注册：<p style=\"font-size: 48px; color:rgb(0, 0, 0);\">" +
                                 std::to_string(code) + "</p>\r\n";

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, from.c_str());
        headers = curl_slist_append(headers, to.c_str());
        std::string payload_text = from + to + type + subject + "\r\n\r\n" + body;

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "1662308219@qq.com");

        struct curl_slist *recipients = NULL;
        recipients = curl_slist_append(recipients, email.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &payload_text);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            LOG_ERROR("错误: %s", curl_easy_strerror(res));

        // free
        curl_slist_free_all(recipients);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}


bool inputAccount(std::string &email,std::string &password)
{
    
}