#include "ChatService.h"

#include <iostream>

#include "base.h"
#include "Neter.h"

void ChatService::sendMessage(std::string &sender_id, std::string &reciver_id, std::string &content)
{
    json sendInfo;
    sendInfo["msgid"] = CHAT_MSG;
    sendInfo["sender_id"] = sender_id;
    sendInfo["reciver_id"] = reciver_id;
    sendInfo["content"] = content;
    neter_->sendJson(sendInfo);
}