#include "groupmodel.hpp"
#include "db/db.h"
// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s','%s')",
            group.getGroupName().c_str(), group.getGroupDesc().c_str());
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 3.设置插入成功的用户数据生成的主键id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
// 加入群组
void GroupModel::joinGroup(int userid, int groupid, string role)
{
    // 1.组床SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d,%d,'%s')",
        groupid, userid, role.c_str());
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select a.* from allgroup a inner join groupuser b on b.groupid = a.id where b.userid = %d", userid);

    vector<Group> vec;
    // 2.获取数据库连接
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 3.遍历查询结果
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    // 查询vector<GroupUser> _users; // 群组成员
    for (Group &group : vec)
    {
        // 1.组装SQL语句
        char sql[1024] = {0};
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on b.userid = a.id where b.groupid = %d", group.getId());
        MySQL mysql;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                // 3.遍历查询结果
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }
    }
    return vec;
}
// 根据指定的groupid查询群组用户信息，除了userid自己，用于用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);

    vector<int> vec;
    // 2.获取数据库连接
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}