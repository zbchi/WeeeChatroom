#include "Service.h"
#include "MySQLConn.h"
#include "base.h"
Service service;
using namespace mylib;
// #include <gperftools/profiler.h>
#include <signal.h>

void signalHandler(int signum)
{
  // std::cout << "\n 捕获到 Ctrl+C (SIGINT)，停止 profiler\n";
  // ProfilerStop();
  exit(signum);
}

int main()
{

  mylib::Logger::setLogLevel(mylib::Logger::INFO);
  // ProfilerStart("cpu.prof");
  ::signal(SIGINT, signalHandler);

  MySQLConnPool::instance().init(32, "127.0.0.1", 3306, "zb", "1662308219@Zb", "chatdb");
  initRedisConnPool();

  service.setNumThreads(4);
  service.start();
  //  ProfilerStop();
}
