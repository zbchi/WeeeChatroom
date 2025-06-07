#include "Controller.h"
#include "Client.h"
#include "unistd.h"

#include <thread>
#include <atomic>

#include <termios.h>
#include <unistd.h>
State state_ = State::LOGINING;
State lastState_ = State::INIT;

int getValidInt(const std::string &prompt)
{
  int value;
  while (true)
  {
    std::cout << prompt;
    std::cin >> value;

    if (std::cin.fail())
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "输入无效" << std::endl;
    }
    else
    {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return value;
    }
  }
}

void Controller::mainLoop()
{

  while (1)
  {
    lastState_ = state_;
    switch (state_)
    {
    case State::REGISTERING:
      showRegister();
      break;

    case State::LOGINING:
      showLogin();
      break;

    case State::LOGGED_IN:
      showMenue();
      break;

    case State::SHOW_FREINDS:
      showFriends();
      break;

    case State::CHAT_FRIEND:
      chatWithFriend();
      break;

    case State::CHAT_GROUP:
      chatWithGroup();
      break;

    case State::ADD_FRIEND:
      showAddFriend();
      break;

    case State::DEL_FRIEND:
      showDelFriend();
      break;

    case State::HANDLE_FRIEND_REQUEST:
      showHandleFriendRequest();
      break;

    case State::CREATE_GROUP:
      showCreateGroup();
      break;

    case State::ADD_GROUP:
      showAddGroup();
      break;

    case State::HANDLE_GROUP_REQUEST:
      showHandleGroupRequest();
      break;

    case State::SHOW_GROUPS:
      showGroups();
      break;
    case State::SHOW_MEMBERS:
      showGroupMembers();
      break;

    case State::EXIT_GROUP:
      showExitGroup();
      break;

    default:
      break;
    }
  }
}

void Controller::showRegister()
{
  sleep(1);
  system("clear");
  std::cout << "REGISTERREGISTERREGISTER" << std::endl;
  std::string email, password, nickname;
  std::cout << "请输入邮箱:";
  std::cin >> email;
  std::cout << "请输入密码:";
  std::cin >> password;
  std::cout << "请输入昵称:";
  std::cin >> nickname;

  client_->userService_.regiSter(email, password, nickname);

  int code;
  while (1)
  {
    int code = getValidInt("请输入验证码:");
    client_->userService_.registerCode(email, password, nickname, code);

    // 阻塞等待服务端回复
    registerWaiter_.wait();
    int reg_errno = registerWaiter_.result;
    if (reg_errno == 0)
    {
      std::cout << "注册成功!" << std::endl;
      state_ = State::LOGINING;
      break;
    }
    else
    {
      std::cout << "错误：" << reg_errno << std::endl;
      if (reg_errno != 1)
      {
        state_ = State::REGISTERING;
        break;
      }
    }
  }
}

void Controller::showLogin()
{
  sleep(1);
  // system("clear");
  std::cout << "LOGININININIIN" << std::endl;

  std::string email, password;
  std::cout << "请输入邮箱:";
  std::cin >> email;
  while (1)
  {
    std::cout << "请输入密码:";
    std::cin >> password;
    client_->userService_.login(email, password);

    // 阻塞等待服务端回复
    loginWaiter_.wait();
    int login_errno = loginWaiter_.result;
    if (login_errno == 0)
    {
      std::cout << "登录成功，用户 : " << client_->user_email_ << std::endl;
      state_ = State::LOGGED_IN;
      break;
    }
    else
    {
      std::cout << "错误:" << login_errno << std::endl;
      if (login_errno != 1)
      {
        state_ = State::LOGINING;
        break;
      }
    }
  }
}

void Controller::flushLogs()
{
  system("clear");
  {
    std::lock_guard<std::mutex> lock(client_->chatService_.chatLogs_mutex_);
    for (auto &chatlog : client_->chatLogs_[client_->currentFriend_.id_])
    {

      std::cout << "[" << chatlog.timestamp << "]";

      if (chatlog.sender_id == client_->user_id_)
        std::cout << "[我]:";
      else
        std::cout << "[" << client_->currentFriend_.nickname_ << "]:";

      std::cout << chatlog.content << std::endl;
    }
  }
}

void Controller::flushRequests()
{
  system("clear");
  int i = 1;
  for (const auto &request : client_->friendRequests_)
  {
    std::cout << i << ".昵称:" << request.nickname_ << "  id:" << request.from_user_id << "  时间:" << request.timestamp_ << std::endl;
    i++;
  }
}

void Controller::flushGroupRequests()
{
  system("clear");
  int i = 1;
  for (const auto &request : client_->groupAddRequests_)
    std::cout << i << ".群名:" << request.group_name << "  用户昵称:" << request.nickname << "  时间" << request.timestamp << std::endl;
}

void Controller::chatWithFriend()
{
  system("clear");
  flushLogs();
  std::string content;
  while (1)
  {
    std::getline(std::cin, content);
    if (content.empty())
      continue;
    ;
    if (content == "/exit")
    {
      state_ = State::LOGGED_IN;
      break;
    }
    client_->chatService_.sendMessage(content);
    flushLogs();
  }
}

void Controller::showAddFriend()
{
  system("clear");
  std::cout << "要加的好友的id:";
  std::string friend_id;
  std::cin >> friend_id;
  client_->friendService_.addFriend(friend_id);
  state_ = State::LOGGED_IN;
}

void Controller::showDelFriend()
{
  system("clear");
  if (client_->friendList_.empty())
  {
    std::cout << "没有可用的好友。" << std::endl;
    return;
  }

  for (size_t i = 0; i < client_->friendList_.size(); ++i)
    std::cout << (i + 1) << ". " << client_->friendList_[i].nickname_ << std::endl;

  std::cout << "请输入要选择的好友编号 (1-" << client_->friendList_.size() << "): ";
  int choice;

  choice = getValidInt("");

  if (choice == 0)
  {
    state_ = State::LOGGED_IN;
    return;
  }

  if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
  {
    std::cout << "无效的选择，请输入 1 到 "
              << client_->friendList_.size() << " 之间的数字。" << std::endl;
    return;
  }

  client_->friendService_.delFriend(client_->friendList_[choice - 1].id_);
  state_ = State::LOGGED_IN;
}

void Controller::showMenue()
{
  // system("clear");
  std::cout << "\n=== 主菜单 ===" << std::endl;
  std::cout << "1. 与好友聊天" << std::endl;
  std::cout << "2. 添加好友" << std::endl;
  std::cout << "3. 处理好友请求" << std::endl;
  std::cout << "4. 退出登录" << std::endl;
  std::cout << "5. 删除好友" << std::endl;
  std::cout << "6. 创建群聊" << std::endl;
  std::cout << "7. 加入群聊" << std::endl;
  std::cout << "8. 处理加群申请" << std::endl;
  std::cout << "9. 展示群聊" << std::endl;
  std::cout << "请输入选项 (1-5): ";

  int choice = getValidInt("请输入选项(1-5):");

  switch (choice)
  {
  case 1:
    state_ = State::SHOW_FREINDS;
    break;
  case 2:
    state_ = State::ADD_FRIEND;
    break;
  case 3:
    state_ = State::HANDLE_FRIEND_REQUEST;
    break;
  case 4:
    state_ = State::LOGINING;
    break;
  case 5:
    state_ = State::DEL_FRIEND;
    break;
  case 6:
    state_ = State::CREATE_GROUP;
    break;
  case 7:
    state_ = State::ADD_GROUP;
    break;
  case 8:
    state_ = State::HANDLE_GROUP_REQUEST;
    break;

  case 9:
    state_ = State::SHOW_GROUPS;
    break;

  default:
    std::cout << "无效选项，请选择 1 到 5。" << std::endl;
    break;
  }
}
void Controller::showFriends()
{
  client_->friendService_.getFriends();
  flushFriends();
  int choice;

  choice = getValidInt("");

  if (choice == 0)
  {
    state_ = State::LOGGED_IN;
    return;
  }

  if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
  {
    std::cout << "无效的选择，请输入 1 到 "
              << client_->friendList_.size() << " 之间的数字。" << std::endl;
    return;
  }

  client_->currentFriend_.setCurrentFriend(client_->friendList_[choice - 1]);
  state_ = State::CHAT_FRIEND;
}

void Controller::showHandleFriendRequest()
{
  system("clear");
  while (1)
  {
    std::lock_guard<std::mutex> lock(client_->friendService_.friendRequests_mutex_);
    flushRequests();
    int i;
    std::cin >> i;
    if (i == 0)
    {
      state_ = State::LOGGED_IN;
      return;
    }

    std::cout << "1.接受  2.拒绝" << std::endl;
    while (1)
    {
      int j;
      std::cin >> j;
      if (j == 0)
      {
        state_ = State::LOGGED_IN;
        break;
      }
      if (j == 1)
        client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "accept");
      else if (j == 2)
        client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "reject");
    }
  }
}

void Controller::showCreateGroup()
{
  std::cout << " 群名:";
  std::string name, description;
  std::cin >> name;
  std::cout << " 描述:";
  std::cin >> description;
  client_->groupService_.createGroup(name, description);
  state_ = State::LOGGED_IN;
}

void Controller::showAddGroup()
{
  std::cout << "要加入的群id:";
  std::string group_id;
  std::cin >> group_id;
  client_->groupService_.addGroup(group_id);
  state_ = State::LOGGED_IN;
}

void Controller::showHandleGroupRequest()
{
  system("clear");
  while (1)
  {
    std::lock_guard<std::mutex> lock(client_->groupService_.groupAddRequests_mutex_);
    flushGroupRequests();
    int i;
    std::cin >> i;
    if (i == 0)
    {
      state_ = State::LOGGED_IN;
      return;
    }

    std::cout << "1.接受  2.拒绝" << std::endl;
    while (1)
    {
      int j;
      std::cin >> j;
      if (j == 0)
      {
        state_ = State::LOGGED_IN;
        break;
      }
      if (j == 1)
        client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "accept");
      else if (j == 2)
        client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "reject");
    }
  }
}

void Controller::flushFriends()
{
  system("clear");
  if (client_->friendList_.empty())
  {
    std::cout << "没有可用的好友。" << std::endl;
    return;
  }

  for (size_t i = 0; i < client_->friendList_.size(); ++i)
  {
    std::cout << (i + 1) << ". " << client_->friendList_[i].nickname_ << "  ";
    if (client_->friendList_[i].isOnline_)
      std::cout << "在线" << std::endl;
    else
      std::cout << "离线" << std::endl;
  }
  std::cout << "请输入要选择的好友编号 (1-" << client_->friendList_.size() << "): " << std::endl;
}

void Controller::showGroups()
{
  client_->groupService_.getGroups();
  flushGroups();
  int choice;

  choice = getValidInt("");

  if (choice == 0)
  {
    state_ = State::LOGGED_IN;
    return;
  }

  if (choice < 1 || choice > static_cast<int>(client_->groupList_.size()))
  {
    std::cout << "无效的选择，请输入 1 到 "
              << client_->groupList_.size() << " 之间的数字。" << std::endl;
    return;
  }

  client_->currentGroup_.setCurrentGroup(client_->groupList_[choice - 1]);
  client_->groupService_.getGroupInfo();
  GroupInfoWaiter_.wait();
  state_ = State::SHOW_MEMBERS;
}

void Controller::flushGroups()
{
  system("clear");
  if (client_->groupList_.empty())
  {
    std::cout << "没有可用的。" << std::endl;
    return;
  }
  for (size_t i = 0; i < client_->groupList_.size(); ++i)
  {
    std::cout << (i + 1) << ". " << client_->groupList_[i].group_name << std::endl;
  }
  std::cout << "请输入要选择的群聊编号 (1-" << client_->groupList_.size() << "): " << std::endl;
}

void Controller::chatWithGroup()
{
  system("clear");
  flushGroupLogs();
  std::string content;
  while (1)
  {
    std::getline(std::cin, content);
    if (content.empty())
      continue;
    ;
    if (content == "/exit")
    {
      state_ = State::LOGGED_IN;
      break;
    }
    client_->chatService_.sendGroupMessage(content);
    flushGroupLogs();
  }
}

void Controller::flushGroupLogs()
{
  system("clear");
  {
    std::lock_guard<std::mutex> lock(client_->chatService_.groupChatLogs_mutex_);
    for (auto &chatlog : client_->groupChatLogs_[client_->currentGroup_.group_id_])
    {

      std::cout << "[" << chatlog.timestamp << "]";

      if (chatlog.sender_id == client_->user_id_)
        std::cout << "[我]:";
      else
        std::cout << "[" << client_->currentGroup_.group_members[chatlog.sender_id].nickname_ << "]:";

      std::cout << chatlog.content << std::endl;
    }
  }
}

void Controller::showGroupMembers()
{
  std::vector<std::string> member_ids;
  int i = 0;
  for (auto &pair : client_->currentGroup_.group_members)
  {
    std::cout << i + 1 << ".名字:" << pair.second.nickname_
              << "  职位:" << pair.second.role_ << "  ID:" << pair.second.id_ << std::endl;
    i++;
    member_ids.push_back(pair.second.id_);
  }
  int choice = getValidInt("");
  if (choice < 1 || choice > static_cast<int>(member_ids.size()))
  {
    std::cout << "无效的编号！" << std::endl;
    return;
  }

  if (client_->currentGroup_.group_members[client_->user_id_].role_ != "member")
  {
    client_->groupService_.kickMember(member_ids[choice - 1]);
  }
  state_ = State::SHOW_GROUPS;
}

void Controller::showExitGroup()
{
  if (client_->currentGroup_.group_members[client_->user_id_].role_ == "owner")
    std::cout << "你是群主，将要解散，确认马" << std::endl;
  else
    std::cout << "将要退出群聊，确认马" << std::endl;

  int choice = getValidInt("");
  if (choice == 1)
    client_->groupService_.exitGroup();
}