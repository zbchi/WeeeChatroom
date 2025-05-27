#include "Client.h"
#include <unistd.h>
#include <signal.h>
Client client;

int main()
{
    signal(SIGCHLD, SIG_IGN);
    client.start();
}