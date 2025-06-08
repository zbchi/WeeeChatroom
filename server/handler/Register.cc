#include "Register.h"

#include "Service.h"
#include "Timestamp.h"
#include "Logger.h"
#include "base.h"

#include "Redis.h"
#include "MySQLConn.h"
#include <curl/curl.h>

void Register::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int code = gVerificationCode();
    std::string email = js["email"].get<std::string>();
    bool isStore = storeCode(email, code);
    bool isSend = sendCode(email, code);
}

void RegisterAcker::handle(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int inputCode = js["code"].get<int>();
    std::string email = js["email"].get<std::string>();
    std::string password = js["password"].get<std::string>();
    std::string nickname = js["nickname"].get<std::string>();

    int errno_verify = verifyCode(email, inputCode);
    if (errno_verify == 0)
    {
        if (inputAccount(email, password, nickname))
            errno_verify = 0;
        else
            errno_verify = -1;
    }
    //-1注册失败   1验证码错误  2该邮箱已注册
    sendJson(conn, makeResponse(REG_MSG_ACK, errno_verify));
}

int RegisterKiter::gVerificationCode()
{
    std::srand(std::time(nullptr));
    int verificationCode = 100000 + (std::rand() % 900000);
    return verificationCode;
}

bool RegisterKiter::storeCode(std::string &email, int code, int expireTime)
{

    std::string key = "verify_email:" + email;

    Redis redis;
    bool isSet = redis.hset(key, "code", std::to_string(code));
    bool isExpire = redis.expire(key, expireTime);
    return isSet && isExpire;
}

int RegisterKiter::verifyCode(std::string &email, int inputCode)
{
    auto mysql = MySQLConnPool::instance().getConnection();
    std::string user_id = mysql->getIdByEmail(email);
    if (user_id != "")
    {
        LOG_DEBUG("%s已经注册", email.c_str());
        return 2;
    }

    std::string key = "verify_email:" + email;
    Redis redis;
    std::string real_code = redis.hget(key, "code");

    if (real_code != std::to_string(inputCode))
    {
        LOG_DEBUG("验证码错误");
        return 1;
    }
    redis.hdel(key, "code");
    LOG_DEBUG("验证成功");
    return 0;
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

bool RegisterKiter::sendCode(std::string &email, int code)
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
        const std::string subject = "Subject: WeeeChatRoom验证码\r\n";
        const std::string body = R"(
<div style="font-family: Arial, sans-serif; padding: 20px;">
  <h2 style="color: #333;">欢迎注册！</h2>
  <p style="font-size: 16px; color: #555;">请使用以下验证码完成注册：</p>
  <div style="margin-top: 20px; padding: 15px; border: 2px dashed #00a2ff; width: fit-content; text-align: center; font-size: 36px; font-weight: bold; color: #00a2ff;">
    )" + std::to_string(code) + R"(</div>
  <p style="margin-top: 20px; font-size: 14px; color: #999;">如果不是您本人操作，请忽略此邮件。</p>
</div>
)";
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

bool RegisterKiter::inputAccount(std::string &email, std::string &password, std::string &nickname)
{

    auto mysql = MySQLConnPool::instance().getConnection();
    bool isSuccess = mysql->insert("users", {{"email", email},
                                             {"nickname", nickname},
                                             {"password", password}});

    if (isSuccess)
        LOG_INFO("注册数据写入成功");
    else
    {
        LOG_ERROR("注册数据写入失败");
        return false;
    }
    return true;
}