#include "db.h"
#include <muduo/base/Logging.h>
// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string database = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, nullptr, 0);
    if (p == nullptr)
    {
        LOG_ERROR << "MySQL connection failed: " << mysql_error(_conn);
        return false;
    }
    // 设置字符集为 UTF-8
    mysql_query(_conn, "set names utf8");
    LOG_INFO << "MySQL connection successful";
    return true;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << "mysql update error: " << mysql_error(_conn);
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << "mysql query error: " << mysql_error(_conn);
        return nullptr;
    }
    return mysql_store_result(_conn);
}

// 获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}
