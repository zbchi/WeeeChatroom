#include "Service.h"
#include "MySQLConn.h"
#include <string>
#include <iostream>

Service service;
using namespace mylib;

int main()
{
  MySQLConnPool::instance().init(10, "127.0.0.1", 3306, "zb", "1662308219@Zb", "chatdb");

  service.start();
}
