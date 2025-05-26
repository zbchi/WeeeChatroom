#include "Controller.h"
#include "Client.h"
#include "unistd.h"

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
    // if (state_ != lastState_)
    //{
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
    case State::CHAT_FRIEND:
      chatWithFriend();
      break;
    default:
      break;
    }
    //}

    while (!client_->messageQueue_.isEmpty())
    {
      std::cout << "-------------------" << std::endl;
      json js = client_->messageQueue_.pop();
      std::string jsonStr = js.dump();
      client_->handleJson(client_->neter_.conn_, jsonStr);
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
    int reg_errno = client_->userService_.registerCode(email, password, nickname, code);

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
        state_ = State::INIT;
        break;
      }
    }
  }
}

void Controller::showLogin()
{
  sleep(1);
  system("clear");
  std::cout << "LOGININININIIN" << std::endl;

  std::string email, password;
  std::cout << "请输入邮箱:";
  std::cin >> email;
  while (1)
  {
    std::cout << "请输入密码:";
    std::cin >> password;
    int login_errno = client_->userService_.login(email, password);

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
        state_ = State::INIT;
        break;
      }
    }
  }
}

void Controller::chatWithFriend()
{
  system("clear");
  for (auto &chatlog : client_->chatLogs_[client_->currentFriend_.id_])
  {
    if (chatlog.sender_id == client_->user_id_)
      std::cout << "[我]:" << chatlog.content << std::endl;
    else
      std::cout << "[" << client_->currentFriend_.nickname_ << "]:" << chatlog.content << std::endl;
  }
  std::cout << "输入框:";
  std::string content;
  std::cin >> content;
  client_->chatService_.sendMessage(client_->user_id_, client_->currentFriend_.id_, content);
}

void Controller::showMenue()
{
  sleep(1);
  system("clear");
  std::cout << "==============主菜单============" << std::endl;
  showFriends();
  int index = 0;
  client_->currentFriend_.setCurrentFriend(client_->firendList_[index]);

  state_ = State::CHAT_FRIEND;
}

void Controller::showFriends()
{
  client_->userService_.getFriends();
  for (const auto &afriend : client_->firendList_)
  {
    int i = 1;
    std::cout << i << "." << afriend.nickname_ << std::endl;
    i++;
  }
}