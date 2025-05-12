#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
class MySQLConn
{
private:
    MYSQL *conn_;

public:
    MySQLConn(/* args */);
    ~MySQLConn();

    bool connect(const std::string &host, int port,
                 const std::string &user, const std::string &password,
                 const std::string &db);
    bool update(const std::string &sql);
    MYSQL_RES *query(const std::string &sql);
    MYSQL *getConnection();
};

class MySQLConnPool
{
private:
    std::queue<MySQLConn *> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    static MySQLConnPool &instance();
    void init(int size, const std::string &host, int port,
              const std::string &user, const std::string &password,
              const std::string &db);
    std::shared_ptr<MySQLConn> getConnection();
};
