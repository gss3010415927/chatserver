#include "friendmodel.hpp"
#include "db/db.h"
// 添加好友关系
void friendModel::insert(int userid, int friendid)
{
    // 1.组床SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d,%d)", userid, friendid);
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 返回用户好友列表  <二表联合查询>
vector<User> friendModel::query(int userid)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User> vec;

    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}