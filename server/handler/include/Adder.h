#include "Handler.h"

using namespace mylib;
class Service;

class AdderFriend : public Handler
{
public:
    AdderFriend(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    int addFriend(std::string &friendEmail, std::string &user_id);
    Service *service_;
};

class AdderGroup : public Handler
{
public:
    AdderGroup(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};