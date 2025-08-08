#pragma once
#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>
using Result = std::vector<std::map<std::string, std::string>>;
class MySQLConn
{
private:
    MYSQL *conn_;
    std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

public:
    MySQLConn();
    ~MySQLConn();
    bool connect(const std::string &host,
                 int port,
                 const std::string &user,
                 const std::string &password,
                 const std::string &db);
    MYSQL *getConnection();

    bool update(const std::string &sql);
    MYSQL_RES *query(const std::string &sql);
    Result queryResult(const std::string &sql);

    bool insert(const std::string &table, const std::map<std::string, std::string> &data);
    bool insertList(const std::string &table, const std::vector<std::map<std::string, std::string>> &dataList);
    bool del(const std::string &table, const std::map<std::string, std::string> &conditions);
    bool update(const std::string &table,
                const std::map<std::string, std::string> &updates,
                const std::map<std::string, std::string> &conditions);
    Result select(const std::string &table,
                  const std::map<std::string, std::string> &conditions = {},
                  const std::map<std::string, std::vector<std::string>> &in_conditions = {},
                  const std::vector<std::pair<std::string, std::string>> &or_conditions = {});

    std::string getEmailById(std::string &user_id);
    std::string getNicknameById(std::string &user_id);
    std::string getIdByEmail(std::string &email);

    int getLastInsertId() { return mysql_insert_id(conn_); }
    std::string escapeStr(const std::string &input);
};

class MySQLConnPool
{
private:
    std::queue<MySQLConn *> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    static MySQLConnPool &instance();
    void init(int size,
              const std::string &host,
              int port,
              const std::string &user,
              const std::string &password,
              const std::string &db);
    std::shared_ptr<MySQLConn> getConnection();
};
