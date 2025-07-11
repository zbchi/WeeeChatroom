#include "Controller.h"
#include "Client.h"
#include <thread>
#include <mutex>
#include <unistd.h>
#include "ui.h"

State state_ = State::LOGINING;

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
        case State::MAIN_MENU:
            showMainMenu();
            break;
        case State::CHAT_PANEL:
            showChatPanel();
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
        case State::FIND_PASSWORD:
            showFindPassword();
            break;
        case State::FILE_FRIEND:
            filePanel();
            break;
        case State::FILE_GROUP:
            filePanel(true);
            break;
        default:
            break;
        }
    }
}

void Controller::showMainMenu()
{
    clearScreen();
    printHeader("🏠 聊天室主页", "欢迎回来！选择您要进行的操作");
    printMenuItem(0, "💬", "消息中心", "查看聊天记录和消息");
    printMenuItem(1, "👥", "好友管理", "添加、删除好友及相关操作");
    printMenuItem(2, "🏢", "群聊管理", "创建群聊、管理群成员");
    printMenuItem(3, "⚙️", "系统设置", "账户设置和退出登录");

    int choice = getValidInt("请选择操作: ");
    switch (choice)
    {
    case 0:
        state_ = State::CHAT_PANEL;
        break;
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
        printStatus("无效选项，请重新选择", "error");
        sleep(1);
        break;
    }
}

void Controller::showChatPanel()
{
    client_->friendService_.getFriends();
    client_->groupService_.getGroups();

    clearScreen();
    printHeader("💬 消息中心", "选择聊天对象开始对话");

    std::vector<std::string> types;
    std::vector<std::string> ids;
    int index = 1;

    // 显示好友列表
    if (!client_->friendList_.empty())
    {
        printDivider("好友列表", "─");
        for (const auto &f : client_->friendList_)
        {
            std::string status = f.isOnline_ ? std::string(SUCCESS) + "● 在线" : std::string(DIM) + "○ 离线";
            std::cout << PRIMARY << "[" << BOLD << index << RESET PRIMARY << "] "
                      << "👤 " << BOLD << f.nickname_ << RESET << " "
                      << status << RESET << "\n";
            types.push_back("friend");
            ids.push_back(f.id_);
            ++index;
        }
        std::cout << "\n";
    }
    // 显示群聊列表
    if (!client_->groupList_.empty())
    {
        printDivider("群聊列表", "─");
        for (const auto &g : client_->groupList_)
        {
            std::cout << SECONDARY << "[" << BOLD << index << RESET SECONDARY << "] "
                      << "🏢 " << BOLD << g.group_name << RESET << "\n";
            types.push_back("group");
            ids.push_back(g.group_id_);
            ++index;
        }
        std::cout << "\n";
    }

    if (index == 1)
    {
        printStatus("暂无好友或群聊，快去添加吧！", "warning");
    }

    // 快捷操作
    printDivider("快捷操作", "=");
    std::cout << WARNING << BOLD << "[91] " << RESET << "📥 好友请求 "
              << BG_WARNING << " " << client_->friendRequests_.size() << " " << RESET << "\n";
    std::cout << INFO << BOLD << "[92] " << RESET << "📨 群聊请求 "
              << BG_PRIMARY << " " << client_->groupAddRequests_.size() << " " << RESET << "\n";
    std::cout << DIM << "[0]  🔙 返回主菜单" << RESET << "\n\n";

    int choice = getValidInt("请选择");

    if (choice == 0)
        state_ = State::MAIN_MENU;
    else if (choice == 91)
        state_ = State::HANDLE_FRIEND_REQUEST;
    else if (choice == 92)
        state_ = State::HANDLE_GROUP_REQUEST;
    else if (choice >= 1 && choice < index)
    {
        if (types[choice - 1] == "friend")
        {
            for (auto &f : client_->friendList_)
            {
                if (f.id_ == ids[choice - 1])
                {
                    client_->currentFriend_.setCurrentFriend(f);
                    state_ = State::CHAT_FRIEND;
                    break;
                }
            }
        }
        else
        {
            for (auto &g : client_->groupList_)
            {
                if (g.group_id_ == ids[choice - 1])
                {
                    client_->currentGroup_.setCurrentGroup(g);
                    client_->groupService_.getGroupInfo();
                    state_ = State::CHAT_GROUP;
                    break;
                }
            }
        }
    }
    else
    {
        printStatus("无效选择", "error");
        sleep(1);
    }
}

void Controller::showFriendMenu()
{
    clearScreen();
    printHeader("👥 好友管理", "管理您的好友关系");

    printMenuItem(1, "💬", "与好友聊天", "选择好友开始对话");
    printMenuItem(2, "➕", "添加好友", "通过ID添加新好友");
    printMenuItem(3, "🗑️", "删除好友", "移除好友关系");
    printMenuItem(4, "📥", "处理好友请求", "查看并处理好友申请");
    printMenuItem(0, "🔙", "返回主菜单", "");

    int choice = getValidInt("请选择操作");
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
        state_ = State::MAIN_MENU;
        break;
    default:
        printStatus("无效选项", "error");
        sleep(1);
        break;
    }
}

void Controller::showGroupMenu()
{
    clearScreen();
    printHeader("🏢 群聊管理", "管理您的群聊");

    printMenuItem(1, "🆕", "创建群聊", "创建一个新的群聊");
    printMenuItem(2, "🔗", "加入群聊", "通过ID加入现有群聊");
    printMenuItem(3, "📨", "处理加群申请", "查看并处理入群请求");
    printMenuItem(4, "📋", "查看群聊列表", "浏览所有已加入的群聊");
    printMenuItem(0, "🔙", "返回主菜单", "");
    int choice = getValidInt("请选择操作:");
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
        state_ = State::MAIN_MENU;
        break;
    default:
        printStatus("无效选项", "error");
        sleep(1);
        break;
    }
}

void Controller::showSystemMenu()
{
    clearScreen();
    printHeader("⚙️ 系统设置", "账户管理和系统设置");

    printMenuItem(1, "🚪", "退出登录", "注销当前账户");
    printMenuItem(2, "🔒", "找回密码", "重置账户密码");
    printMenuItem(0, "🔙", "返回主菜单", "");

    int choice = getValidInt("请选择操作: ");
    switch (choice)
    {
    case 1:
        state_ = State::LOGINING;
        break;
    case 0:
        state_ = State::MAIN_MENU;
        break;
    default:
        printStatus("无效选项", "error");
        sleep(1);
        break;
    }
}

void Controller::showRegister()
{
    clearScreen();
    printHeader("📝 用户注册", "创建您的账户");
    std::string email, password, nickname;

    email = getValidString("📧 请输入邮箱地址: ");
    password = getValidString("🔐 请输入密码: ");
    nickname = getValidString("👤 请输入昵称: ");

    client_->userService_.regiSter(email, password, nickname);

    while (true)
    {
        int code = getValidInt("📩 请输入验证码: ");
        int reg_errno = client_->userService_.registerCode(email, password, nickname, code);

        if (reg_errno == 0)
        {
            printStatus("注册成功！", "success");
            sleep(2);
            state_ = State::LOGINING;
            break;
        }
        else
        {
            printStatus("注册失败，错误码: " + std::to_string(reg_errno), "error");
            if (reg_errno != 1)
            {
                state_ = State::REGISTERING;
                break;
            }
        }
    }
}

void Controller::showFindPassword()
{
    clearScreen();
    printHeader("🔒 找回密码", "重置您的账户密码");
    std::string email, password;
    email = getValidString("📧 请输入账户的邮箱:");
    password = getValidString("🔐请输入新的密码:");
    printStatus("正在发送验证码到您的邮箱...", "info");
    client_->userService_.findPassword(email);

    while (true)
    {
        int code = getValidInt("📩 输入验证码: ");
        int reg_errno = client_->userService_.findPasswordCode(email, password, code);
        if (reg_errno == 0)
        {
            printStatus("密码重置成功!", "success");
            state_ = State::LOGINING;
            break;
        }
        else
        {
            printStatus("重置失败,错误码:" + std::to_string(reg_errno), "error");
            if (reg_errno != 1)
            {
                sleep(1);
                state_ = State::REGISTERING;
                break;
            }
        }
    }
}

void Controller::showLogin()
{
    clearScreen();
    printHeader("🔑 用户登录", "欢迎回来");
    std::string email, password;
    email = getValidString("📧 邮箱地址: ");
    while (true)
    {
        password = getValidString("🔐 密码: ");
        printStatus("正在验证身份...", "info");
        int login_errno = client_->userService_.login(email, password);

        if (login_errno == 0)
        {
            printStatus("登录成功！欢迎 " + client_->user_email_, "success");
            sleep(1);
            state_ = State::CHAT_PANEL;
            break;
        }
        else
        {
            printStatus("登录失败，错误码: " + std::to_string(login_errno), "error");
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
        state_ = State::MAIN_MENU;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
    {
        printStatus("无效编号", "error");
        return;
    }
    client_->currentFriend_.setCurrentFriend(client_->friendList_[choice - 1]);
    state_ = State::CHAT_FRIEND;
}

void Controller::chatWithFriend()
{
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
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
            state_ = State::MAIN_MENU;
            break;
        }
        else if (content == "/c")
        {
            if (client_->chatLogs_[client_->currentFriend_.id_].empty())
            {
                std::cout << "没有更多聊天记录了" << std::endl;
                continue;
            }
            offset += count;
            client_->chatService_.loadMoreChatLogs(client_->currentFriend_.id_, count, offset);
            flushLogs();
            continue;
        }
        else if (content == "/ ")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentFriend_.id_, count, offset);
                flushLogs();
            }
            continue;
        }
        else if (content == "/f")
        {
            state_ = State::FILE_FRIEND;
            break;
        }
        int chat_errno = client_->chatService_.sendMessage(content);
        if (chat_errno == 0)
        {
            client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
            flushLogs();
        }
        else if (chat_errno == 1)
            printStatus("发送失败(你们已不是好友)", "error");
    }
}

void Controller::chatWithGroup()
{
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentGroup_.group_id_, count, true);
    clearScreen();
    flushGroupLogs();
    std::string content;
    while (true)
    {
        std::getline(std::cin, content);
        if (content.empty())
            continue;
        if (content == "/exit")
        {
            state_ = State::MAIN_MENU;
            break;
        }
        else if (content == "/c")
        {
            if (client_->groupChatLogs_[client_->currentGroup_.group_id_].empty())
            {
                std::cout << "没有更多聊天记录了" << std::endl;
                continue;
            }
            offset += count;
            client_->chatService_.loadMoreChatLogs(client_->currentGroup_.group_id_, count, offset, true);
            flushGroupLogs();
            continue;
        }
        else if (content == "/ ")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentGroup_.group_id_, count, offset, true);
                flushGroupLogs();
            }
            continue;
        }

        int chat_errno = client_->chatService_.sendGroupMessage(content);
        if (chat_errno == 0)
        {
            client_->chatService_.loadInitChatLogs(client_->currentGroup_.group_id_, count, true);
            flushGroupLogs();
        }
        else if (chat_errno == 1)
            std::cout << "❌发送失败(你已不在此群聊)" << std::endl;
    }
}

void Controller::showAddFriend()
{
    clearScreen();
    printHeader("➕ 添加好友", "通过用户ID添加新好友");
    std::string friend_id = getValidString("🆔 请输入好友ID: ");
    printStatus("正在发送好友请求...", "info");
    client_->friendService_.addFriend(friend_id);
    printStatus("好友请求已发送！", "success");
    sleep(1);
    state_ = State::MAIN_MENU;
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
        printStatus("无效编号。", "error");
        sleep(1);
        return;
    }
    client_->friendService_.delFriend(client_->friendList_[choice - 1].id_);
    state_ = State::MAIN_MENU;
}

void Controller::showCreateGroup()
{
    clearScreen();
    std::string name, desc;
    name = getValidString("📛 群名:");
    desc = getValidString("📝 群描述: ");
    client_->groupService_.createGroup(name, desc);
    state_ = State::MAIN_MENU;
}

void Controller::showAddGroup()
{
    clearScreen();
    std::string gid = getValidString("🔗 输入要加入的群ID: ");
    printStatus("正在发送加群请求...", "info");
    client_->groupService_.addGroup(gid);
    printStatus("加群请求发送成功。", "success");
    sleep(1);
    state_ = State::MAIN_MENU;
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
            state_ = State::MAIN_MENU;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->friendRequests_.size()))
        {
            printStatus("无效编号", "error");
            sleep(1);
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }
        else if (action == 1)
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "accept");
        else if (action == 2)
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "reject");
        else
        {
            printStatus("无效编号", "error");
            sleep(1);
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
            state_ = State::MAIN_MENU;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->groupAddRequests_.size()))
        {
            printStatus("无效编号", "error");
            sleep(1);
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }
        else if (action == 1)
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "accept");
        else if (action == 2)
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "reject");
        else
        {
            printStatus("无效编号", "error");
            sleep(1);
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
        state_ = State::MAIN_MENU;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->groupList_.size()))
    {
        printStatus("无效编号。", "error");
        return;
    }

    client_->currentGroup_.setCurrentGroup(client_->groupList_[choice - 1]);
    client_->groupService_.getGroupInfo();
    state_ = State::SHOW_MEMBERS;
}
void Controller::showExitGroup()
{
    clearScreen();
    printStatus("确认退出当前群聊?(1=是)", "warning");
    int ch = getValidInt("");
    if (ch == 1)
        client_->groupService_.exitGroup();
    state_ = State::MAIN_MENU;
}

void Controller::showGroupMembers()
{
    clearScreen();
    printHeader("👥 群成员列表");
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

    int choice = getValidInt("🔢 选择成员编号进行管理 (0 返回): ");
    if (choice == 0)
    {
        state_ = State::SHOW_GROUPS;
        return;
    }

    if (choice < 1 || choice > static_cast<int>(member_ids.size()))
    {
        printStatus("无效编号", "error");
        sleep(1);
        state_ = State::SHOW_GROUPS;
        return;
    }

    std::string target_id = member_ids[choice - 1];
    std::string target_role = roles[choice - 1];
    std::string my_role = client_->currentGroup_.group_members[client_->user_id_].role_;

    // 群主 or 管理员才有权限管理他人
    if (my_role == "member")
    {
        printStatus("你没有管理权限。", "error");
        sleep(1);
        state_ = State::SHOW_GROUPS;
        return;
    }
    printMenuItem(1, "❌", "踢出成员");
    printMenuItem(2, "⬆️", "设为管理员");
    printMenuItem(3, "⬇️", "取消管理员");
    printMenuItem(0, "", "返回");
    int action = getValidInt("输入操作编号: ");
    switch (action)
    {
    case 0:
        break;
    case 1:
        client_->groupService_.kickMember(target_id);
        printStatus("已踢出成员", "info");
        break;
    case 2:
        if (target_role == "admin" || target_role == "owner")
            printStatus("对方已经是管理员或群主。", "warning");
        else
        {
            client_->groupService_.addAdmin(target_id);
            printStatus("已设为管理员", "info");
        }
        break;
    case 3:
        if (target_role != "admin")
            printStatus("对方不是管理员，无法取消。", "warning");
        else
        {
            client_->groupService_.removeAdmin(target_id);
            printStatus("已取消管理员。", "info");
        }
        break;
    default:
        printStatus("无效选择", "error");
        sleep(1);
        break;
    }
    state_ = State::SHOW_GROUPS;
}

void Controller::showDestroyGroup()
{
    clearScreen();
    bool isOwner = client_->currentGroup_.group_members[client_->user_id_].role_ == "owner";
    if (isOwner)
        printStatus("你是群主，此操作将解散群聊！", "warning");
    else
        printStatus("你将退出该群聊。", "warning");

    int confirm = getValidInt("确认操作？(1=是): ");
    if (confirm == 1)
    {
        client_->groupService_.exitGroup();
        printStatus("操作已完成。", "success");
    }
    state_ = State::MAIN_MENU;
}

void Controller::flushLogs()
{
    clearScreen();
    printHeader("💬" + client_->currentFriend_.nickname_);
    printLogs(client_->chatLogs_[client_->currentFriend_.id_]);
}

void Controller::flushGroupLogs()
{
    clearScreen();
    printHeader("💬" + client_->currentGroup_.group_name);
    printLogs(client_->groupChatLogs_[client_->currentGroup_.group_id_], true);
}

void Controller::flushFriends()
{
    clearScreen();
    printHeader("好友列表", "选择好友进行操作");
    for (size_t i = 0; i < client_->friendList_.size(); ++i)
        std::cout << (i + 1) << ". " << client_->friendList_[i].nickname_
                  << " [" << (client_->friendList_[i].isOnline_ ? "🟢 在线" : "🔴 离线") << "]\n";
    printInput("🔢 请输入要选择的好友编号 (或 0 返回): ");
}

void Controller::flushGroups()
{
    clearScreen();
    printHeader("🏢群聊列表", "选择群聊进行操作");

    for (size_t i = 0; i < client_->groupList_.size(); ++i)
        std::cout << (i + 1) << ". 📛 " << client_->groupList_[i].group_name << "\n";
    printInput("🔢 请输入要选择的群聊编号 (或 0 返回): ");
}

void Controller::flushRequests()
{
    clearScreen();
    printHeader("📥好友请求列表");
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
    printHeader("📨 群聊请求列表");
    int i = 1;
    for (const auto &req : client_->groupAddRequests_)
    {
        std::cout << i << ". 📛 群: " << req.group_name
                  << " | 👤 用户: " << req.nickname
                  << " | 🕒 时间: " << req.timestamp << "\n";
        ++i;
    }
}

void Controller::printLogs(ChatLogs &chatLogs, bool is_group)
{
    const int boxWidth = 60;
    for (const auto &log : chatLogs)
    {
        std::string time = log.timestamp;
        std::string sender;
        if (is_group)
            sender = log.sender_id == client_->user_id_ ? "我" : client_->currentGroup_.group_members[log.sender_id].nickname_;
        else
            sender = log.sender_id == client_->user_id_ ? "我" : client_->currentFriend_.nickname_;
        std::string content = log.content;
        std::vector<std::string> lines = wrapContent(content, boxWidth - 2);

        if (log.sender_id == client_->user_id_)
            std::cout << GREEN;
        // 顶部边框
        std::cout << "┌" << repeat(boxWidth, "─") << "┐\n";
        // 昵称 + 时间那一行
        int nameWidth = getDisplayWidth(sender);
        int timeWidth = getDisplayWidth(time);
        int spaceBetween = boxWidth - nameWidth - timeWidth - 2; // 两侧空格
        std::cout << "│ " << sender << std::string(spaceBetween, ' ') << time << " │\n";
        // 消息正文
        for (const auto &line : lines)
        {
            int padding = boxWidth - getDisplayWidth(line) - 2;
            std::cout << "│ " << line << std::string(padding, ' ') << " │\n";
        }
        // 底部边框
        std::cout << "└" << repeat(boxWidth, "─") << "┘\n";

        if (log.sender_id == client_->user_id_)
            std::cout << RESET;
    }
    std::cout << DIM << "💡 提示: /c向上翻页,/ 向下翻页,/f传输文件,/exit退出聊天" << RESET << "\n";
}

void Controller::filePanel(bool is_group)
{
    client_->fileService_.getFiles(is_group);
    flushFiles();
    // printHeader("文件传输");
    std::string input;
    input = getValidString("输入序号下载文件,输入绝对路径上传文件:");
    if (input[0] == '/')
    {
        client_->fileService_.uploadFile(input, is_group);

        // int chat_errno = client_->chatService_.sendMessage(content);
        // if (chat_errno == 1)
        //    printStatus("发送失败(你们已不是好友)", "error");
    }
    else
    {
        int choice = std::stoi(input);
        client_->fileService_.downloadFile(client_->fileList_[choice - 1]);
    }
}

void Controller::flushFiles()
{
    // clearScreen();
    printHeader("文件传输");
    for (size_t i = 0; i < client_->fileList_.size(); ++i)
        std::cout << (i + 1) << ". " << client_->fileList_[i].file_name
                  << client_->fileList_[i].file_size_str << client_->fileList_[i].timestamp << "    " << client_->fileList_[i].sender_id << "   " << client_->fileList_[i].id << "\n";
}
