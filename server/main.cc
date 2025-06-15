#include "Service.h"
#include "MySQLConn.h"

Service service;
using namespace mylib;

int main()
{
  MySQLConnPool::instance().init(10, "127.0.0.1", 3306, "zb", "1662308219@Zb", "chatdb");
  service.setNumThreads(4);
  service.start();
}
