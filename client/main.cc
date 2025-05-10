#include "Client.h"
int main()
{
    InetAddress serverAddr("127.0.0.1", 8000);
    EventLoop loop;
    TcpClient client(&loop, serverAddr);

    Client chat_client;
    client.setConnectionCallback([&chat_client](const TcpConnectionPtr &conn)
                                 { chat_client.reg(conn); });

    client.setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {});

    client.connect();
    loop.loop();
}