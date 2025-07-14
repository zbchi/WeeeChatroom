#include "Client.h"
#include <unistd.h>
#include <signal.h>
#include "Logger.h"
Client client;

int main()
{
    mylib::Logger::setLogLevel(mylib::Logger::FATAL);
    signal(SIGCHLD, SIG_IGN);
    client.start();
}