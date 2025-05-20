#ifndef _USER_H_
#define _USER_H_
#include <string>
using namespace std;

// User表的ORM类
class User
{
public:
    User(int id = -1, string name = "", string password = "", string state = "offline")
        : id(id), name(name), password(password), state(state) {}
    // 设置用户ID
    void setId(int id) { this->id = id; }
    // 设置用户名
    void setName(string name) { this->name = name; }
    // 设置用户密码
    void setPassword(string password) { this->password = password; }
    // 设置用户状态
    void setState(string state) { this->state = state; }
    // 获取用户ID
    int getId() const { return id; }
    // 获取用户名
    string getName() const { return name; }
    // 获取用户密码
    string getPassword() const { return password; }
    // 获取用户状态
    string getState() const { return state; }

protected:
    int id;          // 用户ID
    string name;     // 用户名
    string password; // 用户密码
    string state;    // 用户状态
};

#endif