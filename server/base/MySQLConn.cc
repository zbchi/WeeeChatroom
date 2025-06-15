#include "MySQLConn.h"
#include "Logger.h"
#include <sstream>
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

std::string MySQLConn::escapeStr(const std::string &input)
{
    std::string escape;
    escape.resize(input.size() * 2 + 1);
    unsigned long len = mysql_real_escape_string(conn_, &escape[0], input.c_str(), input.size());
    escape.resize(len);
    return escape;
}

std::string MySQLConn::join(const std::vector<std::string> &vec, const std::string &delimiter)
{
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); i++)
    {
        ss << vec[i];
        if (i < vec.size() - 1)
            ss << delimiter;
    }
    return ss.str();
}

bool MySQLConn::update(const std::string &sql)
{
    if (mysql_query(conn_, sql.c_str()) != 0)
    {
        const char *errorMsg = mysql_error(conn_);
        LOG_ERROR("MySQL UPDATE 失败:%s  执行语句:%s ", errorMsg, sql.c_str());
        return false;
    }
    return true;
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

Result MySQLConn::queryResult(const std::string &sql)
{
    std::vector<std::map<std::string, std::string>> resultVec;
    if (mysql_query(conn_, sql.c_str()) != 0)
    {
        const char *errorMsg = mysql_error(conn_);
        LOG_ERROR("MySQL 查询失败: %s, 执行语句: %s", errorMsg, sql.c_str());
        return resultVec;
    }
    MYSQL_RES *result = mysql_store_result(conn_);
    if (result == nullptr)
    {
        const char *errorMsg = mysql_error(conn_);
        LOG_ERROR("MySQL 获取结果集失败: %s, 执行语句: %s", errorMsg, sql.c_str());
        return resultVec;
    }
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
    auto result = select("users", {{"id", user_id}});
    return result[0]["email"];
}

std::string MySQLConn::getNicknameById(std::string &user_id)
{
    auto result = select("users", {{"id", user_id}});
    return result[0]["nickname"];
}
std::string MySQLConn::getIdByEmail(std::string &email)
{
    auto result = select("users", {{"email", email}});
    if (result.empty())
        return "";
    return result[0]["id"];
}

bool MySQLConn::insert(const std::string &table, const std::map<std::string, std::string> &data)
{
    std::vector<std::string> columns, values;
    for (const auto &pair : data)
    {
        columns.push_back(pair.first);
        values.push_back("'" + escapeStr(pair.second) + "'");
    }
    std::stringstream sql;
    sql << "insert into " << table << " (" << join(columns, ",") << ") values (" << join(values, ",") << ")";
    return update(sql.str());
}

bool MySQLConn::del(const std::string &table, const std::map<std::string, std::string> &conditions)
{
    std::vector<std::string> where;
    for (const auto &pair : conditions)
        where.push_back(pair.first + " = '" + escapeStr(pair.second) + "'");

    std::stringstream sql;
    sql << "delete from " << table << " where " << join(where, " and ");
    return update(sql.str());
}

bool MySQLConn::update(const std::string &table, const std::map<std::string, std::string> &updates,
                       const std::map<std::string, std::string> &conditions)
{
    std::vector<std::string> set;
    for (auto it = updates.begin(); it != updates.end(); ++it)
        set.push_back(it->first + " = '" + escapeStr(it->second) + "'");

    std::stringstream sql;
    sql << "update " << table << " set " << join(set, ", ");

    if (!conditions.empty())
    {
        std::vector<std::string> where_clauses;
        std::stringstream where;
        for (auto it = conditions.begin(); it != conditions.end(); ++it)
        {
            where << it->first << " = '" << escapeStr(it->second) << "'";
            if (std::next(it) != conditions.end())
                where << " and ";
        }
        where_clauses.push_back(where.str());
        sql << " where " << join(where_clauses, " and ");
    }
    return update(sql.str());
}

Result MySQLConn::select(const std::string &table,
                         const std::map<std::string, std::string> &conditions,
                         const std::map<std::string, std::vector<std::string>> &in_conditions)
{
    std::stringstream sql;
    sql << "select * from " << table;

    std::vector<std::string> where_clauses;
    if (!conditions.empty())
    {
        std::stringstream where;
        for (auto it = conditions.begin(); it != conditions.end(); it++)
        {
            where << it->first << " = '" << escapeStr(it->second) << "'";
            if (std::next(it) != conditions.end())
                where << " and ";
        }
        where_clauses.push_back(where.str());
    }

    if (!in_conditions.empty())
    {
        for (const auto &in_condition : in_conditions)
        {
            std::vector<std::string> escape;
            for (const auto &value : in_condition.second)
                escape.push_back("'" + escapeStr(value) + "'");
            where_clauses.push_back(in_condition.first + " in (" + join(escape, ",") + ")");
        }
    }

    if (!where_clauses.empty())
        sql << " where " << join(where_clauses, " and ");
    return queryResult(sql.str());
}