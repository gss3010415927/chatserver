#ifndef _GROUP_H_
#define _GROUP_H_
#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;

class Group
{
public:
    Group(int id = -1, string groupname = "", string groupdesc = "")
        : _id(id), _groupname(groupname), _groupdesc(groupdesc) {}
    // 设置群组id
    void setId(int id) { _id = id; }
    // 设置群组名称
    void setGroupName(string groupname) { _groupname = groupname; }
    // 设置群组描述
    void setGroupDesc(string groupdesc) { _groupdesc = groupdesc; }

    // 获取群组id
    int getId() const { return _id; }
    // 获取群组名称
    string getGroupName() const { return _groupname; }
    // 获取群组描述
    string getGroupDesc() const { return _groupdesc; }
    // 获取群组成员列表
    vector<GroupUser>& getUsers() { return _users; }
private:
    int _id;            // 群组id
    string _groupname;  // 群组名称
    string _groupdesc;  // 群组描述
    vector<GroupUser> _users; // 群组成员
};

#endif