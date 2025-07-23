#include <string>
#include <vector>
#include <iostream>
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"

#define PRIMARY "\033[38;5;75m"    // 亮蓝色
#define SECONDARY "\033[38;5;141m" // 紫色
#define SUCCESS "\033[38;5;82m"    // 绿色
#define WARNING "\033[38;5;220m"   // 黄色
#define ERROR "\033[38;5;196m"     // 红色
#define INFOB "\033[38;5;117m"     // 浅蓝色

#define BG_PRIMARY "\033[48;5;25m"
#define BG_SECONDARY "\033[48;5;236m"
#define BG_SUCCESS "\033[48;5;22m"
#define BG_WARNING "\033[48;5;58m"
#define BG_ERROR "\033[48;5;88m"

#define GRADIENT_1 "\033[38;5;81m"
#define GRADIENT_2 "\033[38;5;117m"
#define GRADIENT_3 "\033[38;5;153m"

#define ARROW_RIGHT "▶"
#define ARROW_LEFT "◀"
#define BULLET "●"
#define DIAMOND "◆"
#define STAR "★"
#define CHECK "✓"
#define CROSS "✗"
#define HEART "♥"

int getDisplayWidth(const std::string &str, int tabSize = 8);
std::string expandTabs(const std::string &str, int tabSize = 8);
void printHeader(const std::string &title, const std::string &subtitle = "");
void printDivider(const std::string &text = "", std::string ch = "-");
void printMenuItem(int index, const std::string &icon, const std::string &text,
                   const std::string &description = "", bool highlight = false);
void printStatus(const std::string &message, const std::string &type = "info");
void printInput(const std::string &prompt, const std::string &icon = "");
int getValidInt(const std::string &prompt);
std::string getValidString(const std::string &prompt, bool echo = true);
std::string getValidStringGetline(const std::string &prompt);
std::vector<std::string> wrapContent(const std::string &text, int maxWidth);
std::string repeat(int count, const std::string &ch);
bool isValidEmail(const std::string &email);
void clearScreen();

inline void printTopBegin()
{
    std::cout << "\033[s\033[1;1H\033[2K\033[1;33m";
    std::cout.flush();
}

inline void printTopEnd()
{
    std::cout << "\033[0m\033[u";
    std::cout.flush();
}
