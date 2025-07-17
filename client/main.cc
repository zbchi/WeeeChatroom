#include "Client.h"
#include <unistd.h>
#include <signal.h>
#include "Logger.h"
#include "ui.h"

#include <gperftools/profiler.h>
void signalHandler(int signum)
{
    std::cout << "\n 捕获到 Ctrl+C (SIGINT)，停止 profiler\n";
    ProfilerStop();
    exit(signum);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::string usage = std::string("用法:") + argv[0] + std::string(" 服务端IP");
        printStatus(usage.c_str(), "error");
        std::exit(0);
    }
    mylib::Logger::setLogLevel(mylib::Logger::DEBUG);
    ProfilerStart("cpu.prof");
    ::signal(SIGINT, signalHandler);

    Client client(argv[1]);

    client.start();
    ProfilerStop();
}
