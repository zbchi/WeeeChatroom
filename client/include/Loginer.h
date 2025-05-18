#include "Handler.h"
class Client;
class Loginer : public Handler
{
public:
    Loginer(Client *client) : client_(client) {}
    void handle(const TcpConnectionPtr &conn, json &js);

private:
    Client *client_;
};