#include "Client.h"
#include <unistd.h>
#include <signal.h>
#include "Logger.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::string usage = std::string("用法:") + argv[0] + std::string(" 服务端IP");
        printStatus(usage.c_str(), "error");
        std::exit(0);
    }
    mylib::Logger::setLogLevel(mylib::Logger::FATAL);
    signal(SIGCHLD, SIG_IGN);
    Client client(argv[1]);
    client.start();
}