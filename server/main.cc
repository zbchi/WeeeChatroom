#include "Service.h"
#include "MySQLConn.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>
Service service;
using namespace mylib;
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
    service.threadPool_.add_task([conn, jsonStr, time]()
                                 { service.handleMessage(conn, jsonStr, time); });
  }
}
int main()
{

  MySQLConnPool::instance().init(10, "127.0.0.1", 3306, "zb", "1662308219@Zb", "chatdb");

  InetAddress listenAddr(8000);
  EventLoop loop;
  TcpServer server(&loop, listenAddr);

  server.setConnectionCallback([](const TcpConnectionPtr &conn) {});
  server.setMessageCallback(onMessage);

  server.start();
  loop.loop();
}
