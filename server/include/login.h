#include <string>
int gVerificationCode();
bool storeCode(std::string &email, int code, int expireTime = 120);
bool sendCode(std::string &email, int code);
bool verifyCode(std::string &email, int inputCode);
