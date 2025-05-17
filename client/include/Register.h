#include "Handler.h"
class Client;
class Register : public Handler
{
public:
    Register(Client *client) : client_(client) {}
    void handle(const TcpConnectionPtr &conn, json &js);

private:
    Client *client_;
};
