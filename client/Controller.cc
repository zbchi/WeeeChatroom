#include "Controller.h"
#include "Client.h"
#include <thread>
#include <mutex>
#include <unistd.h>
#include "ui.h"
#include <filesystem>

std::atomic<State> state_ = State::LOG_OR_REG;

void Controller::mainLoop()
{
    while (true)
    {
        switch (state_)
        {
        case State::LOG_OR_REG:
            showLogOrReg();
            break;
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
        case State::CHAT_FRIEND:
            chatWithFriend();
            break;
        case State::CHAT_GROUP:
            chatWithGroup();
            break;
        case State::ADD_FRIEND:
            showAddFriend();
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
        case State::SHOW_MEMBERS:
            showGroupMembers();
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
        case State::FRIEND_PANEL:
            friendPanel();
            break;
        case State::GROUP_PANEL:
            groupPanel();
            break;
        case State::DESTROY_ACCOUNT:
            showDestroyAccount();
            break;
        default:
            break;
        }
    }
}

void Controller::showMainMenu()
{
    clearScreen();
    printHeader("🏠 聊天室主页", "选择您要进行的操作");
    printMenuItem(1, "➕", "添加好友", "通过ID添加新好友");
    printMenuItem(2, "🆕", "创建群聊", "创建一个新的群聊");
    printMenuItem(3, "🔗", "加入群聊", "通过ID加入现有群聊");
    printMenuItem(4, "  ", "注销账户", "彻底销毁当前账户");
    std::cout << DIM << "[ESC]  🔙 返回" << RESET << "\n\n";
    std::string choice = getValidString("请选择操作: ");
    if (choice == "ESC")
    {
        state_ = State::CHAT_PANEL;
        return;
    }
    else if (choice == "1")
        state_ = State::ADD_FRIEND;
    else if (choice == "2")
        state_ = State::CREATE_GROUP;
    else if (choice == "3")
        state_ = State::ADD_GROUP;
    else if (choice == "4")
        state_ = State::DESTROY_ACCOUNT;
    else
    {
        printStatus("无效选项，请重新选择", "error");
        sleep(1);
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

    std::lock_guard<std::mutex> lock1(client_->friendListMutex_);
    std::lock_guard<std::mutex> lock2(client_->groupListMutex_);
    // 显示好友列表
    if (!client_->friendList_.empty())
    {
        printDivider("好友列表", "─");
        for (const auto &[id, f] : client_->friendList_)
        {
            std::string status = f.isOnline_ ? std::string(SUCCESS) + "● 在线" : std::string(DIM) + "○ 离线";
            std::string is_unread = client_->isReadMap_[f.id_] ? std::string(ERROR) + " ● 💬 " : "";
            std::cout << PRIMARY << "[" << BOLD << index << RESET PRIMARY << "] "
                      << "👤 " << BOLD << f.nickname_ << RESET << " "
                      << status << "  " << is_unread << RESET << "\n";
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
        for (const auto &[id, g] : client_->groupList_)
        {
            std::string is_unread = client_->isReadGroupMap_[g.group_id_] ? std::string(ERROR) + " ● 💬 " : "";
            std::cout << SECONDARY << "[" << BOLD << index << RESET SECONDARY << "] "
                      << "🏢 " << BOLD << g.group_name << "[" << g.group_id_ << "]" << "  " << is_unread << RESET << "\n";
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
    std::cout << INFOB << BOLD << "[92] " << RESET << "📨 群聊请求 "
              << BG_PRIMARY << " " << client_->groupAddRequests_.size() << " " << RESET << "\n";
    std::cout << DIM << "[ESC]  🔙 返回主菜单" << RESET << "\n\n";

    std::string choice_str = getValidString("请选择");

    if (choice_str == "ESC")
    {
        state_ = State::MAIN_MENU;
        return;
    }
    int choice;
    try
    {
        choice = std::stoi(choice_str);
    }
    catch (const std::exception &e)
    {
        printStatus("输入无效。", "error");
        sleep(1);
        return;
    }
    if (choice == 91)
        state_ = State::HANDLE_FRIEND_REQUEST;
    else if (choice == 92)
        state_ = State::HANDLE_GROUP_REQUEST;
    else if (choice >= 1 && choice < index)
    {
        if (types[choice - 1] == "friend")
        {
            for (auto &[id, f] : client_->friendList_)
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
            for (auto &[id, g] : client_->groupList_)
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

void Controller::showLogOrReg()
{
    clearScreen();
    printHeader("登录或注册");
    printMenuItem(1, "登录", "");
    printMenuItem(2, "注册", "");
    printMenuItem(3, "找回密码", "");
    int choice = getValidInt("请选择操作：");
    switch (choice)
    {
    case 1:
        state_ = State::LOGINING;
        break;
    case 2:
        state_ = State::REGISTERING;
        break;
    case 3:
        state_ = State::FIND_PASSWORD;
        break;
    default:
        printStatus("无效选项，请重新选择", "error");
        sleep(1);
        break;
    }
}

void Controller::showRegister()
{
    clearScreen();
    printHeader("📝 用户注册(ESC返回)", "创建您的账户");
    std::string email, password, nickname;

    email = getValidString("📧 请输入邮箱地址: ");
    if (email == "ESC")
    {
        state_ = State::LOG_OR_REG;
        return;
    }
    password = getValidString("🔐 请输入密码: ", false);
    if (password == "ESC")
    {
        state_ = State::LOG_OR_REG;
        return;
    }
    nickname = getValidStringGetline("👤 请输入昵称(ESC+回车返回): ");
    if (nickname.back() == 27)
    {
        state_ = State::LOG_OR_REG;
        return;
    }

    client_->userService_.regiSter(email, password, nickname);

    while (true)
    {
        std::string code_str = getValidString("📩 请输入验证码: ");
        if (code_str == "ESC")
        {
            state_ == State::LOG_OR_REG;
            return;
        }
        int code;
        try
        {
            code = std::stoi(code_str);
        }
        catch (const std::exception &e)
        {
            printStatus("输入无效。", "error");
            sleep(1);
            continue;
        }

        int reg_errno = client_->userService_.registerCode(email, password, nickname, code);

        if (reg_errno == 0)
        {
            printStatus("注册成功！", "success");
            sleep(1);
            state_ = State::LOGINING;
            break;
        }
        else if (reg_errno == 1)
        {
            printStatus("验证码错误", "error");
            // sleep(1);
        }
        else if (reg_errno == 2)
        {
            printStatus("该邮箱已注册", "error");
            sleep(1);
            state_ = State::REGISTERING;
            break;
        }
    }
}

void Controller::showFindPassword()
{
    clearScreen();
    printHeader("🔒 找回密码(ESC返回)", "重置您的账户密码");
    std::string email, password;
    email = getValidString("📧 请输入账户的邮箱:");
    if (email == "ESC")
    {
        state_ = State::LOG_OR_REG;
        return;
    }
    password = getValidString("🔐请输入新的密码:", false);
    if (password == "ESC")
    {
        state_ = State::LOG_OR_REG;
        return;
    }
    printStatus("正在发送验证码到您的邮箱...", "info");
    client_->userService_.findPassword(email);

    while (true)
    {
        std::string code_str = getValidString("📩 请输入验证码: ");
        if (code_str == "ESC")
        {
            state_ == State::LOG_OR_REG;
            return;
        }
        int code;
        try
        {
            code = std::stoi(code_str);
        }
        catch (const std::exception &e)
        {
            printStatus("输入无效。", "error");
            sleep(1);
            continue;
        }
        int reg_errno = client_->userService_.findPasswordCode(email, password, code);
        if (reg_errno == 0)
        {
            printStatus("密码重置成功!", "success");
            state_ = State::LOGINING;
            sleep(1);
            break;
        }
        else if (reg_errno == 1)
        {
            printStatus("验证码错误", "error");
            // sleep(1);
        }
        else if (reg_errno == 2)
        {
            printStatus("该邮箱未注册", "error");
            state_ = State::LOG_OR_REG;
            sleep(1);
            break;
        }
    }
}

void Controller::showLogin()
{
    clearScreen();
    printHeader("🔑 用户登录(ESC返回)", "欢迎回来");
    std::string email, password;
    email = getValidString("📧 邮箱地址: ");
    if (email == "ESC")
    {
        state_ = State::LOG_OR_REG;
        return;
    }
    while (true)
    {
        password = getValidString("🔐 密码: ", false);
        if (password == "ESC")
        {
            state_ = State::LOG_OR_REG;
            return;
        }
        printStatus("正在验证身份...", "info");
        int login_errno = client_->userService_.login(email, password);

        if (login_errno == 0)
        {
            printStatus("登录成功！欢迎 " + client_->user_email_, "success");
            sleep(1);
            state_ = State::CHAT_PANEL;
            break;
        }
        else if (login_errno == 3)
        {
            printStatus(email + "已经登录", "error");
            sleep(1);
            break;
        }
        else if (login_errno == 2)
        {
            printStatus(email + "未注册", "error");
            sleep(1);
            break;
        }
        else if (login_errno == 1)
        {
            printStatus("密码错误", "error");
            // sleep(1);
        }
    }
}

void Controller::chatWithFriend()
{
    // 清空未读状态
    {
        std::lock_guard<std::mutex> lock(client_->isReadMapMutex_);
        client_->isReadMap_[client_->currentFriend_.id_] = false;
    }
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
    flushLogs();
    std::string content;
    while (true)
    {
        content = getValidStringGetline("");
        if (content.empty())
            continue;
        if (content.back() == 27)
        {
            state_ = State::CHAT_PANEL;
            break;
        }
        else if (content == "/c")
        {
            state_ = State::LOG_HISTORY;
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
        else if (content == "/v")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentFriend_.id_, count, offset);
                flushLogs();
            }
            else if (offset == 0)
                state_ = State::CHAT_FRIEND;
            continue;
        }
        else if (content == "/f")
        {
            state_ = State::FILE_FRIEND;
            break;
        }
        else if (content == "/m")
        {
            state_ = State::FRIEND_PANEL;
            break;
        }
        int chat_errno = client_->chatService_.sendMessage(content);
        if (chat_errno == 0)
        {
            if (state_ == State::LOG_HISTORY)
            { // 查看历史消息时发送回到底部
                offset = 0;
                client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
                flushLogs();
                state_ = State::CHAT_FRIEND;
            }
        }
        else if (chat_errno == 1)
            printStatus("发送失败(你们已不是好友)", "error");
        else if (chat_errno == 2)
            printStatus("发送失败(对方拒收了)", "error");
    }
}

void Controller::chatWithGroup()
{
    // 清空未读状态
    {
        std::lock_guard<std::mutex> lock(client_->isReadGroupMapMutex_);
        client_->isReadGroupMap_[client_->currentGroup_.group_id_] = false;
    }
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentGroup_.group_id_, count, true);
    flushGroupLogs();
    std::string content;
    while (true)
    {
        content = getValidStringGetline("");
        if (content.empty())
            continue;
        if (content.back() == 27)
        {
            state_ = State::CHAT_PANEL;
            break;
        }
        else if (content == "/c")
        {
            state_ = State::LOG_HISTORY;
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
        else if (content == "/v")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentGroup_.group_id_, count, offset, true);
                flushGroupLogs();
            }
            else if (offset == 0)
                state_ = State::CHAT_FRIEND;
            continue;
        }
        else if (content == "/f")
        {
            state_ = State::FILE_GROUP;
            break;
        }
        else if (content == "/m")
        {
            state_ = State::GROUP_PANEL;
            break;
        }
        int chat_errno = client_->chatService_.sendGroupMessage(content);
        if (chat_errno == 0)
        {
            if (state_ == State::LOG_HISTORY)
            { // 查看历史消息时发送回到底部
                client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
                flushLogs();
                state_ = State::CHAT_FRIEND;
            }
        }
        else if (chat_errno == 1)
            printStatus("发送失败(你已不在群聊)", "error");
    }
}

void Controller::showAddFriend()
{
    clearScreen();
    printHeader("➕ 添加好友", "通过邮箱添加新好友");
    std::string friend_id = getValidString(" 请输入好友邮箱: ");
    if (friend_id == "ESC")
    {
        state_ = State::MAIN_MENU;
        return;
    }
    printStatus("正在发送好友请求...", "info");
    int add_errno = client_->friendService_.addFriend(friend_id);
    if (add_errno == 0)
        printStatus("好友请求已发送！", "success");
    else if (add_errno == 1)
        printStatus("该用户未注册。", "error");
    else if (add_errno == 2)
        printStatus("不能添加自己。", "error");
    else if (add_errno == 3)
        printStatus("你们已经是好友关系。", "success");
    else if (add_errno == 4)
        printStatus("已经申请过(待对方处理)", "info");
    sleep(1);
    state_ = State::MAIN_MENU;
}

void Controller::showCreateGroup()
{
    clearScreen();
    printHeader("创建群聊(ESC+回车返回)", "");
    std::string name, desc;
    name = getValidStringGetline("📛 群名:");
    if (name.back() == 27)
    {
        state_ = State::MAIN_MENU;
        return;
    }
    desc = getValidString("📝 群描述: ");
    if (desc.back() == 27)
    {
        state_ = State::MAIN_MENU;
        return;
    }
    client_->groupService_.createGroup(name, desc);
    state_ = State::MAIN_MENU;
}

void Controller::showAddGroup()
{
    clearScreen();
    printHeader("添加群聊", "");
    std::string gid = getValidString("🔗 输入要加入的群ID: ");
    if (gid == "ESC")
    {
        state_ = State::MAIN_MENU;
        return;
    }
    printStatus("正在发送加群请求...", "info");
    int add_errno = client_->groupService_.addGroup(gid);
    if (add_errno == 0)
        printStatus("加群请求发送成功！", "success");
    else if (add_errno == 1)
        printStatus("不存在该群聊。", "error");
    else if (add_errno == 2)
        printStatus("你已经在此群聊。", "success");
    else if (add_errno == 3)
        printStatus("已经申请过(待群主或管理员处理)", "info");
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
        std::string choice_str = getValidString("请选择(ESC返回)");

        if (choice_str == "ESC")
        {
            state_ = State::CHAT_PANEL;
            return;
        }
        int choice;
        try
        {
            choice = std::stoi(choice_str);
        }
        catch (const std::exception &e)
        {
            printStatus("输入无效。", "error");
            sleep(1);
            return;
        }

        if (choice < 1 || choice > static_cast<int>(client_->friendRequests_.size()))
        {
            printStatus("无效编号", "error");
            sleep(1);
            continue;
        }
        printMenuItem(1, "✅ 接受", "");
        printMenuItem(2, "❌ 拒绝", "");
        printMenuItem(0, "?  返回", "");

        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::HANDLE_FRIEND_REQUEST;
            return;
        }
        else if (action == 1)
            client_->friendService_.responseFriendRequest(client_->friendRequests_[choice - 1], "accept");
        else if (action == 2)
            client_->friendService_.responseFriendRequest(client_->friendRequests_[choice - 1], "reject");
        else if (action == 0)
            continue;
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
        flushGroupRequests();

        std::string choice_str = getValidString("请选择(ESC返回)");

        if (choice_str == "ESC")
        {
            state_ = State::CHAT_PANEL;
            return;
        }
        int choice;
        try
        {
            choice = std::stoi(choice_str);
        }
        catch (const std::exception &e)
        {
            printStatus("输入无效。", "error");
            sleep(1);
            return;
        }

        if (choice < 1 || choice > static_cast<int>(client_->groupAddRequests_.size()))
        {
            printStatus("无效编号", "error");
            sleep(1);
            continue;
        }

        printMenuItem(1, "✅ 接受", "");
        printMenuItem(2, "❌ 拒绝", "");
        printMenuItem(0, "?  返回", "");

        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::HANDLE_GROUP_REQUEST;
            return;
        }
        else if (action == 1)
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[choice - 1], "accept");
        else if (action == 2)
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[choice - 1], "reject");
        else if (action == 0)
            continue;
        else
        {
            printStatus("无效编号", "error");
            sleep(1);
        }
    }
}

void Controller::showGroupMembers()
{
    clearScreen();
    printHeader("👥 群成员列表");

    std::vector<std::string> member_ids;
    std::vector<std::string> roles;

    // 使用现有的样式常量
    std::cout << BOLD << PRIMARY << "序号  昵称                    角色        用户ID" << RESET << "\n";
    printDivider("", "─");

    int i = 0;
    for (const auto &pair : client_->currentGroup_.group_members)
    {
        std::string displayName = pair.second.nickname_;
        if (getDisplayWidth(displayName) > 20)
        {

            while (getDisplayWidth(displayName + "...") > 20 && !displayName.empty())
                displayName.pop_back();
            displayName += "...";
        }

        std::string displayRole = pair.second.role_;
        if (getDisplayWidth(displayRole) > 10)
        {
            while (getDisplayWidth(displayRole + "...") > 10 && !displayRole.empty())
            {
                displayRole.pop_back();
            }
            displayRole += "...";
        }

        int nameWidth = getDisplayWidth(displayName);
        int roleWidth = getDisplayWidth(displayRole);

        printf("%-4d  %-*s%*s  %-*s%*s  %s\n",
               i + 1,
               (int)displayName.length(), displayName.c_str(), 20 - nameWidth, "",
               (int)displayRole.length(), displayRole.c_str(), 10 - roleWidth, "",
               pair.second.id_.c_str());

        member_ids.push_back(pair.second.id_);
        roles.push_back(pair.second.role_);
        ++i;
    }
    std::cout << "\n";

    int choice = getValidInt("🔢 选择成员编号进行管理 (0 返回): ");
    if (choice == 0)
    {
        state_ = State::GROUP_PANEL;
        return;
    }

    if (choice < 1 || choice > static_cast<int>(member_ids.size()))
    {
        printStatus("无效编号", "error");
        sleep(1);
        state_ = State::GROUP_PANEL;
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
        if (target_id == client_->user_id_)
            printStatus("不能踢自己。", "error");
        else if (target_role == "owner")
            printStatus("不能踢群主。", "error");
        else if (target_role == "admin" && my_role == "admin")
            printStatus("无权限踢其他管理员。", "error");
        else
        {
            printStatus("已踢出成员", "success");
            client_->groupService_.kickMember(target_id);
        }
        sleep(1);
        break;
    case 2:
        if (target_role == "admin" || target_role == "owner")
            printStatus("对方已经是管理员或群主。", "warning");
        else
        {
            client_->groupService_.addAdmin(target_id);
            printStatus("已设为管理员", "success");
        }
        sleep(1);
        break;
    case 3:
        if (target_role != "admin")
            printStatus("对方不是管理员，无法取消。", "warning");
        else
        {
            client_->groupService_.removeAdmin(target_id);
            printStatus("已取消管理员。", "success");
        }
        sleep(1);
        break;
    default:
        printStatus("无效选择", "error");
        sleep(1);
        break;
    }
    state_ = State::GROUP_PANEL;
}

void Controller::showDestroyGroup()
{
    clearScreen();
    printHeader("退出/解散群聊");
    bool isOwner = client_->currentGroup_.group_members[client_->user_id_].role_ == "owner";
    if (isOwner)
        printStatus("你是群主，此操作将解散群聊！", "warning");
    else
        printStatus("你将退出该群聊。", "warning");

    int confirm = getValidInt("确认操作？(1=是,0=否): ");
    if (confirm == 1)
    {
        client_->groupService_.exitGroup();
        printStatus("操作已完成。", "success");
        sleep(1);
        state_ = State::CHAT_PANEL;
    }
    else if (confirm == 0)
        state_ = State::CHAT_GROUP;
}

void Controller::showDestroyAccount()
{
    clearScreen();
    printHeader("销毁账户", "");
    printStatus("你将销毁账户。", "warning");

    std::string confirm = getValidString("确认操作？(1=是,0=否): ");
    if (confirm == "1")
    {
        client_->userService_.destroyAccount();
        printStatus("操作已完成。", "success");
        sleep(1);
        state_ = State::LOGINING;
    }
    else if (confirm == "0")
        state_ = State::MAIN_MENU;
    else if (confirm == "ESC")
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

void Controller::flushRequests()
{
    clearScreen();
    printHeader("📥 好友请求列表");

    std::cout << BOLD << PRIMARY << "序号  昵称                用户ID           请求时间" << RESET << "\n";

    for (size_t i = 0; i < client_->friendRequests_.size(); ++i)
    {
        const auto &req = client_->friendRequests_[i];
        std::string displayNick = req.nickname_;

        if (getDisplayWidth(displayNick) > 18)
        {
            while (getDisplayWidth(displayNick + "...") > 18 && !displayNick.empty())
                displayNick.pop_back();
            displayNick += "...";
        }
        int nickWidth = getDisplayWidth(displayNick);

        printf("%-4zu  %-*s%*s  %-16s  %-s\n",
               i + 1,
               (int)displayNick.length(), displayNick.c_str(), 18 - nickWidth, "",
               req.from_user_id.c_str(),
               req.timestamp_.c_str());
    }

    std::cout << "\n";
}

void Controller::flushGroupRequests()
{
    clearScreen();
    printHeader("📨 群聊请求列表");

    std::cout << BOLD << PRIMARY << "序号  群聊名                     用户昵称            请求时间" << RESET << "\n";
    std::lock_guard lock(client_->groupService_.groupAddRequests_mutex_);

    for (size_t i = 0; i < client_->groupAddRequests_.size(); ++i)
    {
        const auto &req = client_->groupAddRequests_[i];

        std::string displayGroup = req.group_name;
        std::string displayNick = req.nickname;

        if (getDisplayWidth(displayGroup) > 26)
        {
            while (getDisplayWidth(displayGroup + "...") > 26 && !displayGroup.empty())
                displayGroup.pop_back();
            displayGroup += "...";
        }

        if (getDisplayWidth(displayNick) > 18)
        {
            while (getDisplayWidth(displayNick + "...") > 18 && !displayNick.empty())
                displayNick.pop_back();
            displayNick += "...";
        }

        int groupWidth = getDisplayWidth(displayGroup);
        int nickWidth = getDisplayWidth(displayNick);

        printf("%-4zu  %-*s%*s  %-*s%*s  %-s\n",
               i + 1,
               (int)displayGroup.length(), displayGroup.c_str(), 26 - groupWidth, "",
               (int)displayNick.length(), displayNick.c_str(), 18 - nickWidth, "",
               req.timestamp.c_str());
    }

    std::cout << "\n";
}

void Controller::printLogs(ChatLogs &chatLogs, bool is_group)
{
    for (const auto &log : chatLogs)
        printALog(log, is_group);

    std::cout << DIM << "💡 提示: /c向上翻页,/v向下翻页,/f传输文件,/m管理聊天,ESC+回车退出聊天" << RESET << "\n";
}

void Controller::printALog(const ChatMessage &log, bool is_group)
{
    const int boxWidth = 60;
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

void Controller::filePanel(bool is_group)
{
    namespace fs = std::filesystem;
    client_->fileService_.getFiles(is_group);
    flushFiles(is_group);
    // printHeader("文件传输");
    std::string input = getValidStringGetline("输入序号下载文件,输入绝对路径上传文件,ESC+回车返回上级:");
    if (input.back() == 27)
    {
        state_ = is_group ? State::CHAT_GROUP : State::CHAT_FRIEND;
        return;
    }
    if (!input.empty() && input[0] == '/')
    {
        if (!fs::exists(input))
        {
            printStatus("该文件不存在。", "error");
            sleep(1);
            return;
        }
        if (!fs::is_regular_file(input))
        {
            printStatus("不是普通文件或无法访问。", "error");
            sleep(1);
            return;
        }
        int up_errno = client_->fileService_.uploadFile(input, is_group);
        if (up_errno == 1)
            printStatus("发送失败(无关系)", "error");
        else if (up_errno == 2)
            printStatus("发送失败(被对方拒收了)", "error");
        else if (up_errno == 0)
            printStatus("开始发送文件(后台传输)", "success");
        sleep(1);
        // state_ = is_group ? State::CHAT_GROUP : State::CHAT_FRIEND;
        //  int chat_errno = client_->chatService_.sendMessage(content);
        //  if (chat_errno == 1)
        //     printStatus("发送失败(你们已不是好友)", "error");
    }
    else
    {
        int choice = -1;
        try
        {
            choice = std::stoi(input);
        }
        catch (const std::exception &e)
        {
            printStatus("输入无效，应该为编号或绝对路径", "error");
            sleep(1);
            return;
        }

        if (choice < 1 || choice > static_cast<int>(client_->fileList_.size()))
        {
            printStatus("无效编号", "error");
            sleep(1);
            return;
        }
        client_->fileService_.downloadFile(client_->fileList_[choice - 1]);
    }
}

void Controller::flushFiles(bool is_group)
{
    clearScreen();
    printHeader("文件传输");
    std::cout << BOLD << PRIMARY << "序号  文件名                          文件大小    发送时间            发送者" << RESET << "\n";

    for (size_t i = 0; i < client_->fileList_.size(); ++i)
    {
        const auto &file = client_->fileList_[i];
        std::string sender = is_group ? client_->currentGroup_.group_members[file.sender_id].nickname_ : file.sender_id;

        std::string displayName = file.file_name;
        if (getDisplayWidth(displayName) > 30)
        {

            while (getDisplayWidth(displayName + "...") > 30 && !displayName.empty())
                displayName.pop_back();
            displayName += "...";
        }

        std::string displaySender = sender;
        if (getDisplayWidth(displaySender) > 10)
        {
            while (getDisplayWidth(displaySender + "...") > 10 && !displaySender.empty())
                displaySender.pop_back();
            displaySender += "...";
        }

        int nameWidth = getDisplayWidth(displayName);
        int senderWidth = getDisplayWidth(displaySender);

        printf("%-4zu  %-*s%*s  %-10s  %-18s  %-*s%*s\n",
               i + 1,
               (int)displayName.length(), displayName.c_str(), 30 - nameWidth, "",
               file.file_size_str.c_str(),
               file.timestamp.c_str(),
               (int)displaySender.length(), displaySender.c_str(), 10 - senderWidth, "");
    }

    std::cout << "\n";
}

void Controller::friendPanel()
{
    clearScreen();
    std::string head = "好友ID:" + client_->currentFriend_.id_ + "  昵称:" + client_->currentFriend_.nickname_;
    printHeader(head.c_str());
    printMenuItem(1, "删掉Ta", "你们将结束好友关系");
    printMenuItem(2, "屏蔽Ta", "你将拒收此好友的消息");
    printMenuItem(3, "解除屏蔽", "不再拒收此好友的消息");

    std::string choice = getValidString("请选择操作：");
    if (choice == "ESC")
        state_ = State::CHAT_FRIEND;
    else if (choice == "1")
    {
        client_->friendService_.delFriend(client_->currentFriend_.id_);
        state_ = State::CHAT_PANEL;
        printStatus("删除好友成功", "success");
        sleep(1);
    }
    else if (choice == "2")
    {
        int block_errno = client_->friendService_.blockFriend(client_->currentFriend_.id_);
        state_ = State::CHAT_FRIEND;
        if (block_errno == 0)
            printStatus("屏蔽好友成功", "success");
        else if (block_errno == 1)
            printStatus("你已经将此好友屏蔽", "error");
        sleep(1);
    }
    else if (choice == "3")
    {
        int unblock_errno = client_->friendService_.unblockFriend(client_->currentFriend_.id_);
        state_ = State::CHAT_FRIEND;
        if (unblock_errno == 0)
            printStatus("解除屏蔽此好友成功", "success");
        else if (unblock_errno == 1)
            printStatus("你未将存此好友屏蔽", "error");
        sleep(1);
    }
    else
    {
        printStatus("无效编号", "error");
        sleep(1);
    }
}

void Controller::groupPanel()
{
    client_->groupService_.getGroupInfo();
    clearScreen();
    std::string head = "群聊ID:" + client_->currentGroup_.user_id_ + "  群名:" + client_->currentGroup_.group_name;
    printHeader(head.c_str());
    printMenuItem(1, "查看群成员列表", "");
    printMenuItem(2, "退出群/解散群", "");
    std::string choice = getValidString("请选择操作：");
    if (choice == "ESC")
        state_ = State::CHAT_GROUP;
    else if (choice == "1")
        state_ = State::SHOW_MEMBERS;
    else if (choice == "2")
        state_ = State::DESTORY_GROUP;
    else
    {
        printStatus("无效选项，请重新选择", "error");
        sleep(1);
    }
}