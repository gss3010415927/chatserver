#ifndef _GROUPMODEL_H_
#define _GROUPMODEL_H_
#include "group.hpp"
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    void joinGroup(int userid, int groupid, string role);
    // 查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    // 根据指定的groupid查询群组用户信息，除了userid自己，用于用户群聊业务给群组其他成员群发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif