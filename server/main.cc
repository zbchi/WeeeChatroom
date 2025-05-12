#include "Service.h"
#include "MySQLConn.h"
#include <string>
#include <iostream>

int main()
{

  MySQLConnPool::instance().init(10, "127.0.0.1", 3306, "zb", "1662308219@Zb", "chatdb");

  InetAddress listenAddr(8000);
  EventLoop loop;
  TcpServer server(&loop, listenAddr);

  Service service;

  server.setConnectionCallback([](const TcpConnectionPtr &conn) {});
  server.setMessageCallback([&service](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                            {std:: string msg =buf->retrieveAsString();
                              std::cout<<msg<<std::endl;
                            service.handleMessage(conn,msg,time); });

  server.start();
  loop.loop();
}
