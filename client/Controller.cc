#include "Controller.h"
#include "Client.h"
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <mutex>

#include <unistd.h>
State state_ = State::LOGINING;
void clearScreen()
{
    system("clear");
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
            std::cout << "❌ 输入无效，请输入数字。\n";
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
    while (true)
    {
        switch (state_)
        {
        case State::REGISTERING:
            showRegister();
            break;
        case State::LOGINING:
            showLogin();
            break;
        case State::LOGGED_IN:
            showMainMenu();
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

        case State::DESTORY_GROUP:
            showDestroyGroup();
            break;

        default:
            break;
        }
    }
}

void Controller::showMainMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        主菜单          ║
╚════════════════════════╝

[1] 👤 好友相关功能
[2] 👥 群聊相关功能
[3] ⚙️ 系统设置/退出
)";
    int choice = getValidInt("请输入选项 (1-3): ");
    switch (choice)
    {
    case 1:
        showFriendMenu();
        break;
    case 2:
        showGroupMenu();
        break;
    case 3:
        showSystemMenu();
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showFriendMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║     👤 好友功能菜单     ║
╚════════════════════════╝

[1] 与好友聊天
[2] 添加好友
[3] 删除好友
[4] 处理好友请求
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::SHOW_FREINDS;
        break;
    case 2:
        state_ = State::ADD_FRIEND;
        break;
    case 3:
        state_ = State::DEL_FRIEND;
        break;
    case 4:
        state_ = State::HANDLE_FRIEND_REQUEST;
        break;
    case 0:
        state_ = State::LOGGED_IN;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showGroupMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║     👥 群聊功能菜单     ║
╚════════════════════════╝

[1] 创建群聊
[2] 加入群聊
[3] 处理加群申请
[4] 查看群聊列表
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::CREATE_GROUP;
        break;
    case 2:
        state_ = State::ADD_GROUP;
        break;
    case 3:
        state_ = State::HANDLE_GROUP_REQUEST;
        break;
    case 4:
        state_ = State::SHOW_GROUPS;
        break;
    case 0:
        state_ = State::LOGGED_IN;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showSystemMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║      ⚙️ 系统设置菜单     ║
╚════════════════════════╝

[1] 退出登录
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::LOGINING;
        break;
    case 0:
        state_ = State::LOGGED_IN;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showRegister()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       📝 用户注册       ║
╚════════════════════════╝
)";
    std::string email, password, nickname;
    std::cout << "📧 邮箱: ";
    std::cin >> email;
    std::cout << "🔐 密码: ";
    std::cin >> password;
    std::cout << "👤 昵称: ";
    std::cin >> nickname;

    client_->userService_.regiSter(email, password, nickname);

    while (true)
    {
        int code = getValidInt("📩 输入验证码: ");
        client_->userService_.registerCode(email, password, nickname, code);

        registerWaiter_.wait();
        int reg_errno = registerWaiter_.result;
        if (reg_errno == 0)
        {
            std::cout << "✅ 注册成功!\n";
            state_ = State::LOGINING;
            break;
        }
        else
        {
            std::cout << "❌ 注册失败，错误码: " << reg_errno << "\n";
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
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       🔑 用户登录       ║
╚════════════════════════╝
)";
    std::string email, password;
    std::cout << "📧 邮箱: ";
    std::cin >> email;

    while (true)
    {
        std::cout << "🔐 密码: ";
        std::cin >> password;
        client_->userService_.login(email, password);

        loginWaiter_.wait();
        int login_errno = loginWaiter_.result;
        if (login_errno == 0)
        {
            std::cout << "✅ 登录成功，欢迎 " << client_->user_email_ << "\n";
            state_ = State::LOGGED_IN;
            break;
        }
        else
        {
            std::cout << "❌ 登录失败，错误码: " << login_errno << "\n";
            if (login_errno != 1)
            {
                state_ = State::LOGINING;
                break;
            }
        }
    }
}

void Controller::showFriends()
{
    client_->friendService_.getFriends();
    flushFriends();
    int choice = getValidInt("");
    if (choice == 0)
    {
        state_ = State::LOGGED_IN;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }
    client_->currentFriend_.setCurrentFriend(client_->friendList_[choice - 1]);
    state_ = State::CHAT_FRIEND;
}

void Controller::chatWithFriend()
{
    clearScreen();
    std::cout << "💬 与好友聊天（输入 /exit 退出）\n";
    flushLogs();
    std::string content;
    while (true)
    {
        std::getline(std::cin, content);
        if (content.empty())
            continue;
        if (content == "/exit")
        {
            state_ = State::LOGGED_IN;
            break;
        }
        client_->chatService_.sendMessage(content);
        flushLogs();
    }
}

void Controller::chatWithGroup()
{
    clearScreen();
    std::cout << "💬 群聊中（输入 /exit 退出）\n";
    flushGroupLogs();
    std::string content;
    while (true)
    {
        std::getline(std::cin, content);
        if (content.empty())
            continue;
        if (content == "/exit")
        {
            state_ = State::LOGGED_IN;
            break;
        }
        client_->chatService_.sendGroupMessage(content);
        flushGroupLogs();
    }
}

void Controller::showAddFriend()
{
    clearScreen();
    std::cout << "➕ 请输入要添加的好友ID: ";
    std::string friend_id;
    std::cin >> friend_id;
    client_->friendService_.addFriend(friend_id);
    state_ = State::LOGGED_IN;
}

void Controller::showDelFriend()
{
    clearScreen();
    if (client_->friendList_.empty())
    {
        std::cout << "⚠️ 当前没有好友。\n";
        return;
    }
    std::cout << "👥 好友列表:\n";
    for (size_t i = 0; i < client_->friendList_.size(); ++i)
        std::cout << i + 1 << ". " << client_->friendList_[i].nickname_ << "\n";
    int choice = getValidInt("🔢 选择要删除的好友编号: ");
    if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }
    client_->friendService_.delFriend(client_->friendList_[choice - 1].id_);
    state_ = State::LOGGED_IN;
}

void Controller::showCreateGroup()
{
    clearScreen();
    std::string name, desc;
    std::cout << "📛 群名: ";
    std::cin >> name;
    std::cout << "📝 群描述: ";
    std::cin >> desc;
    client_->groupService_.createGroup(name, desc);
    state_ = State::LOGGED_IN;
}

void Controller::showAddGroup()
{
    clearScreen();
    std::string gid;
    std::cout << "🔗 输入要加入的群ID: ";
    std::cin >> gid;
    client_->groupService_.addGroup(gid);
    state_ = State::LOGGED_IN;
}
void Controller::showHandleFriendRequest()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(client_->friendService_.friendRequests_mutex_);
            flushRequests();
        }

        int i = getValidInt("🔢 选择请求编号 (0 返回): ");
        if (i == 0)
        {
            state_ = State::LOGGED_IN;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->friendRequests_.size()))
        {
            std::cout << "❌ 无效编号\n";
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::LOGGED_IN;
            return;
        }
        else if (action == 1)
        {
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "accept");
        }
        else if (action == 2)
        {
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "reject");
        }
        else
        {
            std::cout << "❌ 无效操作\n";
        }
    }
}

void Controller::showHandleGroupRequest()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(client_->groupService_.groupAddRequests_mutex_);
            flushGroupRequests();
        }

        int i = getValidInt("🔢 选择请求编号 (0 返回): ");
        if (i == 0)
        {
            state_ = State::LOGGED_IN;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->groupAddRequests_.size()))
        {
            std::cout << "❌ 无效编号\n";
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::LOGGED_IN;
            return;
        }
        else if (action == 1)
        {
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "accept");
        }
        else if (action == 2)
        {
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "reject");
        }
        else
        {
            std::cout << "❌ 无效操作\n";
        }
    }
}

void Controller::showGroups()
{
    client_->groupService_.getGroups();
    flushGroups();
    int choice = getValidInt("");
    if (choice == 0)
    {
        state_ = State::LOGGED_IN;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->groupList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }

    client_->currentGroup_.setCurrentGroup(client_->groupList_[choice - 1]);
    client_->groupService_.getGroupInfo();
    GroupInfoWaiter_.wait();
    state_ = State::SHOW_MEMBERS;
}
void Controller::showExitGroup()
{
    clearScreen();
    std::cout << "⚠️ 确认退出当前群聊？(1=是): ";
    int ch = getValidInt("");
    if (ch == 1)
        client_->groupService_.exitGroup();
    state_ = State::LOGGED_IN;
}

void Controller::showGroupMembers()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════════════╗
║        👥 群成员列表           ║
╚════════════════════════════════╝
)";
    std::vector<std::string> member_ids;
    std::vector<std::string> roles;
    int i = 0;
    for (const auto &pair : client_->currentGroup_.group_members)
    {
        std::cout << i + 1 << ". 👤 " << pair.second.nickname_
                  << " | 🏷 角色: " << pair.second.role_
                  << " | 🆔: " << pair.second.id_ << "\n";
        member_ids.push_back(pair.second.id_);
        roles.push_back(pair.second.role_);
        ++i;
    }

    if (member_ids.empty())
    {
        std::cout << "⚠️ 没有成员\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    int choice = getValidInt("🔢 选择成员编号进行管理 (0 返回): ");
    if (choice == 0)
    {
        state_ = State::SHOW_GROUPS;
        return;
    }

    if (choice < 1 || choice > static_cast<int>(member_ids.size()))
    {
        std::cout << "❌ 无效编号\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    std::string target_id = member_ids[choice - 1];
    std::string target_role = roles[choice - 1];
    std::string my_role = client_->currentGroup_.group_members[client_->user_id_].role_;

    // 群主 or 管理员才有权限管理他人
    if (my_role == "member")
    {
        std::cout << "🚫 你没有管理权限。\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    std::cout << R"(
选择操作:
1. ❌ 踢出成员
2. ⬆️ 设为管理员
3. ⬇️ 取消管理员
0. 返回
)";
    int action = getValidInt("输入操作编号: ");
    switch (action)
    {
    case 0:
        break;
    case 1:
        client_->groupService_.kickMember(target_id);
        std::cout << "✅ 已踢出成员。\n";
        break;
    case 2:
        if (target_role == "admin" || target_role == "owner")
        {
            std::cout << "⚠️ 对方已经是管理员或群主。\n";
        }
        else
        {
            client_->groupService_.addAdmin(target_id);
            std::cout << "✅ 已设为管理员。\n";
        }
        break;
    case 3:
        if (target_role != "admin")
        {
            std::cout << "⚠️ 对方不是管理员，无法取消。\n";
        }
        else
        {
            client_->groupService_.removeAdmin(target_id);
            std::cout << "✅ 已取消管理员。\n";
        }
        break;
    default:
        std::cout << "❌ 无效操作。\n";
        break;
    }

    state_ = State::SHOW_GROUPS;
}

void Controller::showDestroyGroup()
{
    clearScreen();
    bool isOwner = client_->currentGroup_.group_members[client_->user_id_].role_ == "owner";
    if (isOwner)
    {
        std::cout << "⚠️ 你是群主，此操作将解散群聊！\n";
    }
    else
    {
        std::cout << "⚠️ 你将退出该群聊。\n";
    }
    int confirm = getValidInt("确认操作？(1=是): ");
    if (confirm == 1)
    {
        client_->groupService_.exitGroup();
        std::cout << "✅ 操作已完成。\n";
    }
    state_ = State::LOGGED_IN;
}

void Controller::flushLogs()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       聊天记录         ║
╚════════════════════════╝
)";
    std::lock_guard<std::mutex> lock(client_->chatService_.chatLogs_mutex_);
    for (const auto &log : client_->chatLogs_[client_->currentFriend_.id_])
    {
        std::cout << "[" << log.timestamp << "] ";
        if (log.sender_id == client_->user_id_)
            std::cout << "🧑‍💻 我: ";
        else
            std::cout << client_->currentFriend_.nickname_ << ": ";
        std::cout << log.content << "\n";
    }
}

void Controller::flushGroupLogs()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       群聊记录         ║
╚════════════════════════╝
)";
    std::lock_guard<std::mutex> lock(client_->chatService_.groupChatLogs_mutex_);
    for (const auto &log : client_->groupChatLogs_[client_->currentGroup_.group_id_])
    {
        std::cout << "[" << log.timestamp << "] ";
        if (log.sender_id == client_->user_id_)
            std::cout << "🧑‍💻 我: ";
        else
            std::cout << client_->currentGroup_.group_members[log.sender_id].nickname_ << ": ";
        std::cout << log.content << "\n";
    }
}

void Controller::flushFriends()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        好友列表        ║
╚════════════════════════╝
)";
    if (client_->friendList_.empty())
    {
        std::cout << "⚠️ 没有好友。\n";
        return;
    }

    for (size_t i = 0; i < client_->friendList_.size(); ++i)
    {
        std::cout << (i + 1) << ". " << client_->friendList_[i].nickname_
                  << " [" << (client_->friendList_[i].isOnline_ ? "🟢 在线" : "🔴 离线") << "]\n";
    }
    std::cout << "🔢 请输入要选择的好友编号 (或 0 返回): ";
}

void Controller::flushRequests()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║      好友请求列表      ║
╚════════════════════════╝
)";
    int i = 1;
    for (const auto &req : client_->friendRequests_)
    {
        std::cout << i << ". 👤 昵称: " << req.nickname_
                  << " | 🆔: " << req.from_user_id
                  << " | 🕒 时间: " << req.timestamp_ << "\n";
        ++i;
    }
}

void Controller::flushGroupRequests()
{
    clearScreen();
    std::cout << R"(
╔══════════════════════════════╗
║        群聊加群请求列表      ║
╚══════════════════════════════╝
)";
    int i = 1;
    for (const auto &req : client_->groupAddRequests_)
    {
        std::cout << i << ". 📛 群: " << req.group_name
                  << " | 👤 用户: " << req.nickname
                  << " | 🕒 时间: " << req.timestamp << "\n";
        ++i;
    }
}

void Controller::flushGroups()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        群聊列表        ║
╚════════════════════════╝
)";
    if (client_->groupList_.empty())
    {
        std::cout << "⚠️ 当前没有加入任何群聊。\n";
        return;
    }

    for (size_t i = 0; i < client_->groupList_.size(); ++i)
    {
        std::cout << (i + 1) << ". 📛 " << client_->groupList_[i].group_name << "\n";
    }

    std::cout << "🔢 请输入要选择的群聊编号 (或 0 返回): ";
}
