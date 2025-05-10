#include "Service.h"

#include <string>
#include <iostream>

int main()
{
  InetAddress listenAddr(8000);
  EventLoop loop;
  TcpServer server(&loop, listenAddr);

  Service service;

  server.setConnectionCallback([](const TcpConnectionPtr &conn) {});
  server.setMessageCallback([&service](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                            {std:: string msg =buf->retrieveAsString();
                              std::cout<<msg;
                            service.handleMessage(conn,msg,time); });

  server.start();
  loop.loop();
}
