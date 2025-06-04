#include "GroupService.h"

#include "base.h"
#include "Neter.h"
#include "Client.h"

void GroupService::createGroup(std::string &groupname, std::string &description)
{
    json groupInfo;
    groupInfo["msgid"] = CREATE_GROUP;
    groupInfo["creator_id"] = client_->user_id_;
    groupInfo["name"] = groupname;
    groupInfo["description"] = description;
    neter_->sendJson(groupInfo);
}
