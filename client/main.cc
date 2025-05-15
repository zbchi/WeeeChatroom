#include "Client.h"
#include <unistd.h>
Client client;

int main()
{
    client.start();
    while (1)
    {
        sleep(1);
    }
}