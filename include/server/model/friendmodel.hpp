#ifndef _FRIENDMODEL_H_
#define _FRIENDMODEL_H_

#include <vector>
#include "user.hpp"
using namespace std;
// 维护好友信息的操作类
class friendModel
{
public:
    // 添加好友关系
    void insert(int userid, int friendid);
    // 返回用户好友列表  <二表联合查询>
    vector<User> query(int userid);
};

#endif