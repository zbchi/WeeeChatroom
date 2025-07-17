#include "ui.h"
#include <iostream>
#include <iomanip>
#include <limits>

std::string repeat(int count, const std::string &ch)
{
    std::string result;
    for (int i = 0; i < count; ++i)
        result += ch;
    return result;
}

int getDisplayWidth(const std::string &str)
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
        else if ((c >> 5) == 0x6)
        { // 2-byte UTF-8
            width += 2;
            i += 2;
        }
        else if ((c >> 4) == 0xE)
        { // 3-byte UTF-8
            width += 2;
            i += 3;
        }
        else if ((c >> 3) == 0x1E)
        { // 4-byte UTF-8
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
        color = INFO;
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

std::string getValidString(const std::string &prompt)
{
    std::string value;
     printInput(prompt);
    if (!std::getline(std::cin, value))
        std::exit(0);
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
    {
        lines.push_back(currentLine);
    }

    return lines;
}

void clearScreen()
{
    system("clear");
}