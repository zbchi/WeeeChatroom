#include "ui.h"
#include <iostream>
#include <iomanip>
#include <limits>

#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>

#include <termios.h>
#include <fcntl.h>

#include <regex>

#include <readline/readline.h>

std::string repeat(int count, const std::string &ch)
{
    std::string result;
    for (int i = 0; i < count; ++i)
        result += ch;
    return result;
}

int getDisplayWidth(const std::string &str, int tabSize)
{
    int width = 0;
    for (size_t i = 0; i < str.size();)
    {
        unsigned char c = str[i];

        if (c < 0x80)
        {
            width += 1;
            i += 1;
        }
        else if ((c >> 5) == 0x6) // 2-byte UTF-8
        {
            width += 2;
            i += 2;
        }
        else if ((c >> 4) == 0xE) // 3-byte UTF-8
        {
            width += 2;
            i += 3;
        }
        else if ((c >> 3) == 0x1E) // 4-byte UTF-8
        {
            width += 2;
            i += 4;
        }
        else
        {
            width += 1;
            i += 1;
        }
    }
    return width;
}

void printHeader(const std::string &title, const std::string &subtitle)
{
    const int totalWidth = 69; // 框内宽度
    std::cout << "\n"
              << PRIMARY << BOLD;
    std::cout << "╭─────────────────────────────────────────────────────────────────────╮\n";

    int titleWidth = getDisplayWidth(title);
    int titlePadding = (totalWidth - titleWidth) / 2;
    std::cout << "│" << std::string(titlePadding, ' ') << title
              << std::string(totalWidth - titlePadding - titleWidth, ' ') << "│\n";

    if (!subtitle.empty())
    {
        int subtitleWidth = getDisplayWidth(subtitle);
        int subtitlePadding = (totalWidth - subtitleWidth) / 2;
        std::cout << "│" << DIM << std::string(subtitlePadding, ' ') << subtitle
                  << std::string(totalWidth - subtitlePadding - subtitleWidth, ' ')
                  << RESET << PRIMARY << BOLD << "│\n";
    }

    std::cout << "╰─────────────────────────────────────────────────────────────────────╯" << RESET << "\n\n";
}

void printDivider(const std::string &text, std::string ch)
{
    int width = 70;
    if (text.empty())
    {
        std::cout << DIM << repeat(width, ch) << RESET << "\n";
    }
    else
    {
        int textLen = getDisplayWidth(text);
        int leftPad = (width - textLen - 2) / 2;
        int rightPad = width - textLen - 2 - leftPad;
        std::cout << DIM << repeat(leftPad, ch) << " " << RESET
                  << SECONDARY << BOLD << text << RESET << DIM
                  << " " << repeat(rightPad, ch) << RESET << "\n";
    }
}

void printMenuItem(int index, const std::string &icon, const std::string &text,
                   const std::string &description, bool highlight)
{
    std::string color = highlight ? std::string(BG_PRIMARY) + PRIMARY : PRIMARY;
    std::string resetColor = highlight ? RESET : "";

    std::cout << color;
    if (highlight)
        std::cout << " ";

    std::cout << "[" << GRADIENT_1 << BOLD << index << RESET << color << "] "
              << icon << " " << BOLD << text << RESET;

    if (!description.empty())
    {
        std::cout << color << DIM << " - " << description << RESET;
    }

    if (highlight)
        std::cout << " ";
    std::cout << resetColor << "\n";
}

void printStatus(const std::string &message, const std::string &type)
{
    std::string color, icon;
    if (type == "success")
    {
        color = SUCCESS;
        icon = CHECK;
    }
    else if (type == "error")
    {
        color = ERROR;
        icon = CROSS;
    }
    else if (type == "warning")
    {
        color = WARNING;
        icon = "⚠";
    }
    else
    {
        color = INFOB;
        icon = "ℹ";
    }

    std::cout << "\n"
              << color << BOLD << icon << " " << message << RESET << "\n\n";
}

void printInput(const std::string &prompt, const std::string &icon)
{
    std::cout << GRADIENT_2 << BOLD << icon << " " << prompt << RESET;
}

int getValidInt(const std::string &prompt)
{
    int value;
    while (true)
    {
        printInput(prompt);
        std::cin >> value;
        if (std::cin.fail())
        {
            if (std::cin.eof())
                std::exit(0);
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printStatus("输入无效，请输入数字", "error");
        }
        else
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}
struct termios orig_termios;
void saveOriginalTerios()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
}

void setRawMode()
{
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restoreMode()
{ // 恢复原始设置
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void clearStdinBuffer()
{
    // 将 STDIN 设置为非阻塞
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    char dump[64];
    while (read(STDIN_FILENO, dump, sizeof(dump)) > 0)
    {
    }
    fcntl(STDIN_FILENO, F_SETFL, flags);
}

std::string getValidString(const std::string &prompt, bool echo)
{
    printInput(prompt);
    std::cout.flush();
    std::string value;
    tcflush(STDIN_FILENO, TCIFLUSH);
    setRawMode();
    char ch;
    while (read(STDIN_FILENO, &ch, 1) == 1)
    {
        if (ch == 3) // Ctrl+C
        {
            restoreMode();
            std::cout << "^C\n";
            std::exit(0);
        }
        else if (ch == 27)
        {
            restoreMode();
            std::cout << "\n";
            return "ESC";
        }
        else if (ch == '\r' || ch == '\n')
        { // 回车键
            restoreMode();
            std::cout << "\n";
            return value;
        }
        else if (ch == 127 || ch == 8)
        { // 退格键
            if (!value.empty())
            {
                value.pop_back();
                if (echo)
                    std::cout << "\b \b"; // 只有开启回显时才删除显示
                else
                    std::cout << "\b \b"; // 密码模式也需要处理光标位置
                std::cout.flush();
            }
        }
        else if (ch >= 32 && ch <= 126)
        { // 可打印字符
            value += ch;
            if (echo)
                std::cout << ch; // 正常显示字符
            else
                std::cout << "●";
            std::cout.flush();
        }
    }
    restoreMode();
    return value;
}

std::string getValidStringGetline(const std::string &prompt, bool is_limit)
{
    std::string value;
    printInput(prompt);
    if (!std::getline(std::cin, value))
        std::exit(0);
    if (is_limit)
    {
        if (value.size() > 64)
            value.resize(64);
    }
    return value;
}

std::string getValidStringReadline(const std::string &prompt)
{
    char *input = readline(prompt.c_str());
    if (!input) // Ctrl+D
        return "";
    std::string value(input);
    free(input);
    return value;
}

std::vector<std::string> wrapContent(const std::string &text, int maxWidth)
{
    std::vector<std::string> lines;
    std::string currentLine;
    int currentWidth = 0;

    for (size_t i = 0; i < text.size();)
    {
        unsigned char c = text[i];
        int charLen = 1;
        int charWidth = 1;

        if (c < 0x80)
        {
            charLen = 1;
            charWidth = 1;
        }
        else if ((c >> 5) == 0x6)
        {
            charLen = 2;
            charWidth = 2;
        }
        else if ((c >> 4) == 0xE)
        {
            charLen = 3;
            charWidth = 2;
        }
        else if ((c >> 3) == 0x1E)
        {
            charLen = 4;
            charWidth = 2;
        }
        if (currentWidth + charWidth > maxWidth)
        {
            lines.push_back(currentLine);
            currentLine.clear();
            currentWidth = 0;
        }
        currentLine += text.substr(i, charLen);
        currentWidth += charWidth;
        i += charLen;
    }
    if (!currentLine.empty())
        lines.push_back(currentLine);
    return lines;
}

std::string expandTabs(const std::string &str, int tabSize)
{
    std::string result;
    int col = 0;
    for (size_t i = 0; i < str.size();)
    {
        unsigned char c = str[i];
        if (c == '\t')
        {
            int spaces = tabSize - (col % tabSize);
            result.append(spaces, ' ');
            col += spaces;
            i += 1;
        }
        else if (c < 0x80)
        {
            result.push_back(c);
            col += 1;
            i += 1;
        }
        else if ((c >> 5) == 0x6)
        {
            result.append(str.substr(i, 2));
            col += 2;
            i += 2;
        }
        else if ((c >> 4) == 0xE)
        {
            result.append(str.substr(i, 3));
            col += 2;
            i += 3;
        }
        else if ((c >> 3) == 0x1E)
        {
            result.append(str.substr(i, 4));
            col += 2;
            i += 4;
        }
        else
        {
            result.push_back(c);
            col += 1;
            i += 1;
        }
    }
    return result;
}

bool isValidEmail(const std::string &email)
{
    const std::regex pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, pattern);
}

void clearScreen()
{
    system("clear");
}

void disableKeys()
{
    rl_bind_key('\t', rl_insert); // Tab
    rl_unbind_key(1);             // Ctrl+A
    rl_unbind_key(5);             // Ctrl+E
    rl_unbind_key(11);            // Ctrl+K
    rl_unbind_key(21);            // Ctrl+U
    rl_unbind_key(23);            // Ctrl+W
    rl_unbind_key(12);            // Ctrl+L
    rl_unbind_key(18);            // Ctrl+R
    rl_unbind_key(20);            // Ctrl+T
}

std::string fixInvalidUtf8(const std::string &input)
{
    std::string result;
    size_t i = 0;
    while (i < input.size())
    {
        unsigned char c = static_cast<unsigned char>(input[i]);
        size_t len = 0;

        if (c <= 0x7F)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        else
        {
            i++; // 非法起始字节
            continue;
        }

        if (i + len > input.size())
            break; // 不完整的 UTF-8 末尾

        bool valid = true;
        for (size_t j = 1; j < len; ++j)
        {
            if ((static_cast<unsigned char>(input[i + j]) & 0xC0) != 0x80)
            {
                valid = false;
                break;
            }
        }
        if (valid)
            result.append(input.substr(i, len));

        i += len;
    }
    return result;
}