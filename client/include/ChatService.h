#include <string>
class Neter;
class ChatService
{
public:
    ChatService(Neter *neter) : neter_(neter) {}
    void sendMessage(std::string &sender_id, std::string &reciver_id, std::string &content);

private:
    Neter *neter_;
};