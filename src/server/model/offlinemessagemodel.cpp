#include "offlinemessagemodel.hpp"
#include "db/db.h"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, const string &msg)
{
    // 1.组床SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d,'%s')",
            userid, msg.c_str());
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    // 1.组床SQL语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", userid);
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
        // 1.组床SQL语句
        char sql[1024] = {0};
        sprintf(sql, "select message from offlinemessage where userid = %d", userid);

        vector<string> vec;

        // 2.获取数据库连接
        MySQL mysql;
        if(mysql.connect())
        {
            MYSQL_RES* res = mysql.query(sql);
            if(res != nullptr)
            {
                // 将所有userid对应的离线消息存储到vec中
                MYSQL_ROW row;
                while((row = mysql_fetch_row(res)) != nullptr)
                {
                    vec.push_back(row[0]);
                }
                mysql_free_result(res);
                return vec;
            }
        }
        return vec;
}