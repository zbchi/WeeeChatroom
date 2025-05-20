#include <string>
class Neter;
class UserService
{
public:
    UserService(Neter *neter) : neter_(neter) {}

    void registerCode(std::string &email, std::string &password, std::string &nickname,int code);
    void regiSter(std::string &email, std::string &password, std::string &nickname);
    void login(std::string&email,std::string &password);

private:
    Neter *neter_;
};