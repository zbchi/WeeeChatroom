#include <cstdlib>
#include <ctime>

#include <curl/curl.h>
#include "Logger.h"
#include "login.h"
#include "redis.h"
#include <curl/easy.h>
int gVerificationCode()
{
    std::srand(std::time(nullptr));
    int verificationCode = 100000 + (std::rand() % 900000);
    return verificationCode;
}

bool storeCode(std::string &email, int code, int expireTime)
{

    std::string key = "verify_email:" + email;

    Redis redis;
    bool isSet = redis.hset(key, "code", std::to_string(code));
    bool isExpire = redis.expire(key, expireTime);
    return isSet && isExpire;
}

bool verifyCode(std::string &email, int inputCode)
{
    std::string key = "verify_email:" + email;

    Redis redis;
    std::string real_code = redis.hget(key, "code");

    if (real_code != std::to_string(inputCode))
        return false;

    redis.hdel(key, "code");
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
        LOG_INFO("邮件发送成功");
        return true;
    }
    LOG_ERROR("邮件发送失败");
    return false;
}

bool inputAccount(std::string &email, std::string &password)
{
    LOG_DEBUG("验证成功");
    return true;
}