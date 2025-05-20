#ifndef _GROUPUSER_H_
#define _GROUPUSER_H_
#include <string>
#include "user.hpp"

using namespace std;

// 群组用户，多了一个role角色信息，从User类直接继承，复用User类的其他信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return role; }

private:
    string role; // 群组成员角色
};

#endif