#include "Controller.h"
#include "Client.h"

#include "unistd.h"
void Controller::mainLoop()
{
    std::string email = "1934613109@qq.com";
    std::string password = "1234567890";
    std::string nickname = "winkwink";
    client_->userService_.regiSter(email, password, nickname);
    while (client_->messageQueue_.isEmpty())
    {
        json js = client_->messageQueue_.pop();
        std::string jsonStr = js.dump();
        client_->handleMessage(client_->neter_.conn_, jsonStr);
        sleep(1);
    }
}