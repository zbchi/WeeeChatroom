#include "Handler.h"

using namespace mylib;
class Service;

class RegisterKiter
{
protected:
    static int gVerificationCode();
    static bool storeCode(std::string &email, int code, int expireTime = 120);
    static bool sendCode(std::string &email, int code, bool is_find = false);
    static int verifyCode(std::string &email, int inputCode);

    static bool inputAccount(std::string &email, std::string &password, std::string &nickname);
};

class Register : public Handler, protected RegisterKiter
{
public:
    Register(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class RegisterAcker : public Handler, protected RegisterKiter
{
public:
    RegisterAcker(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class PasswordFinder : public Handler, protected RegisterKiter
{
public:
    PasswordFinder(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class PasswordFindAcker : public Handler, protected RegisterKiter
{
public:
    PasswordFindAcker(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};