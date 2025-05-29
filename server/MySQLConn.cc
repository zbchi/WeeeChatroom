#include "MySQLConn.h"
#include "Logger.h"
MySQLConnPool &MySQLConnPool::instance()
{
    static MySQLConnPool pool;
    return pool;
}

void MySQLConnPool::init(int size, const std::string &host, int port,
                         const std::string &user, const std::string &password,
                         const std::string &db)
{
    for (int i = 0; i < size; i++)
    {
        auto conn = new MySQLConn();
        if (conn->connect(host, port, user, password, db))
        {
            pool_.push(conn);
            LOG_INFO("第 %d 个MySQL连接成功", i + 1);
        }
        else
        {
            delete conn;
            LOG_ERROR("第 %d 个MySQL连接失败", i + 1);
        }
    }
}

std::shared_ptr<MySQLConn> MySQLConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]()
             { return !pool_.empty(); });
    MySQLConn *conn = pool_.front();
    pool_.pop();

    return std::shared_ptr<MySQLConn>(conn, [this](MySQLConn *ptr)
                                      {
                                        std::lock_guard<std::mutex>lock(mutex_);
                                        pool_.push(ptr);
                                        cv_.notify_one(); });
}

MySQLConn::MySQLConn()
{
    conn_ = mysql_init(nullptr);
}
MySQLConn::~MySQLConn()
{
    if (conn_)
        mysql_close(conn_);
}

bool MySQLConn::connect(const std::string &host, int port,
                        const std::string &user, const std::string &password,
                        const std::string &db)
{
    MYSQL *ret = mysql_real_connect(conn_, host.c_str(), user.c_str(),
                                    password.c_str(), db.c_str(), port, nullptr, 0);
    return ret != nullptr;
}

bool MySQLConn::update(const std::string &sql)
{
    return mysql_query(conn_, sql.c_str()) == 0;
}

MYSQL_RES *MySQLConn::query(const std::string &sql)
{
    if (mysql_query(conn_, sql.c_str()) != 0)
        return nullptr;
    return mysql_store_result(conn_);
}

MYSQL *MySQLConn::getConnection()
{
    return conn_;
}

std::vector<std::map<std::string, std::string>> MySQLConn::queryResult(const std::string &sql)
{
    std::vector<std::map<std::string, std::string>> resultVec;
    if (mysql_query(conn_, sql.c_str()) != 0)
        return resultVec;

    MYSQL_RES *result = mysql_store_result(conn_);
    if (result == nullptr)
        return resultVec;

    int numFields = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_field(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::map<std::string, std::string> rowMap;
        for (int i = 0; i < numFields; i++)
        {
            rowMap[fields[i].name] = row[i] ? row[i] : "";
        }
        resultVec.emplace_back(std::move(rowMap));
    }
    mysql_free_result(result);
    return resultVec;
}

std::string MySQLConn::getEmailById(std::string &user_id)
{
    char sql[128];
    snprintf(sql, sizeof(sql), "select email from users where id = '%s'", user_id.c_str());
    auto result = queryResult(std::string(sql));
    if (result.empty())
        return "";
    else
        return result[0]["email"];
}

std::string MySQLConn::getNicknameById(std::string &user_id)
{
    char sql[128];
    snprintf(sql, sizeof(sql), "select nickname from users where id = '%s'", user_id.c_str());
    auto result = queryResult(std::string(sql));
    if (result.empty())
        return "";
    else
        return result[0]["nickname"];
}
std::string MySQLConn::getIdByEmail(std::string &email)
{
    char sql[128];
    snprintf(sql, sizeof(sql), "select id from users where email = '%s'", email.c_str());
    auto result = queryResult(std::string(sql));
    if (result.empty())
        return "";
    else
        return result[0]["id"];
}