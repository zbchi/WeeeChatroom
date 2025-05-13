#include "Client.h"
Client chat_client;

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    while (buf->readableBytes() >= 4)
    {
        const void *data = buf->peek();
        int lenNetOrder;
        memcpy(&lenNetOrder, data, sizeof(lenNetOrder));
        int len = ntohl(lenNetOrder);
        if (buf->readableBytes() < 4 + len)
            break;
        buf->retrieve(4);

        std::string jsonStr(buf->peek(), len);
        buf->retrieve(len);
        std::cout << jsonStr << std::endl;
        chat_client.handleMessage(conn, jsonStr, time);
    }
}
int main()
{
    InetAddress serverAddr("127.0.0.1", 8000);
    EventLoop loop;
    TcpClient client(&loop, serverAddr);

    client.setConnectionCallback([](const TcpConnectionPtr &conn)
                                 { chat_client.reg(conn); });

    client.setMessageCallback(onMessage);

    client.connect();
    loop.loop();
}