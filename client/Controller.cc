#include "Controller.h"
#include "Client.h"
#include "unistd.h"

#include <thread>
#include <atomic>

#include <termios.h>
#include <unistd.h>
State state_ = State::LOGINING;
State lastState_ = State::INIT;

void clearStdin()
{
  tcflush(STDIN_FILENO, TCIFLUSH);
}
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
      std::cout << "输入无效，请输入一个整数！" << std::endl;
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

    case State::ADD_FRIEND:
      showAddFriend();
      break;

    case State::HANDLE_FRIEND_REQUEST:
      showHandleFriendRequest();
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
    {
      std::unique_lock<std::mutex> lock(reg_mtx_);
      regCv_.wait(lock, [this]
                  { return regResultSet_; });
      regResultSet_ = false;
    }

    if (reg_errno_ == 0)
    {
      std::cout << "注册成功!" << std::endl;
      state_ = State::LOGINING;
      break;
    }
    else
    {
      std::cout << "错误：" << reg_errno_ << std::endl;
      if (reg_errno_ != 1)
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
    {
      std::unique_lock<std::mutex> lock(login_mtx_);

      loginCv_.wait(lock, [this]
                    { return loginResultSet_; });
      loginResultSet_ = false;
    }

    if (login_errno_ == 0)
    {
      std::cout << "登录成功，用户 : " << client_->user_email_ << std::endl;
      state_ = State::LOGGED_IN;
      break;
    }
    else
    {
      std::cout << "错误:" << login_errno_ << std::endl;
      if (login_errno_ != 1)
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
  {
    int i = 1;
    for (const auto &request : client_->friendRequests_)
    {
      std::cout << i << ".昵称:" << request.nickname_ << "  id:" << request.from_user_id << "  时间:" << request.timestamp_ << std::endl;
      i++;
    }
  }
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
    client_->chatService_.sendMessage(client_->user_id_, client_->currentFriend_.id_, content);
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

void Controller::showMenue()
{
  system("clear");
  client_->friendService_.getFriends();
  std::cout << "\n=== 主菜单 ===" << std::endl;
  std::cout << "1. 与好友聊天" << std::endl;
  std::cout << "2. 添加好友" << std::endl;
  std::cout << "3. 处理好友请求" << std::endl;
  std::cout << "4. 退出登录" << std::endl;
  std::cout << "请输入选项 (1-4): ";

  int choice;
  if (!(std::cin >> choice))
  {
    std::cout << "输入无效，请输入一个整数。" << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return;
  }

  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
  default:
    std::cout << "无效选项，请选择 1 到 4。" << std::endl;
    break;
  }
}
void Controller::showFriends()
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
  std::cin >> choice;

  if (std::cin.fail() || choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
  {
    std::cout << "无效的选择，请输入 1 到 "
              << client_->friendList_.size() << " 之间的数字。" << std::endl;
    std::cin.clear();                                                   // 清除错误状态
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除输入缓冲区
    return;
  }

  if (choice == 0)
  {
    state_ = State::LOGGED_IN;
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

    while (1)
    {
      int j;
      std::cin >> j;
      if (j == 0)
        break;
      if (j == 1)
        client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "accept");
      else if (j == 2)
        client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "reject");
    }
  }
}