#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前用户的好友信息
vector<User> g_currentUserFriends;
// 记录当前用户的群组信息
vector<Group> g_currentUserGroups;

// 控制主菜单页面程序
bool isMainMennuRunning = false;

// 定义用于读写线程之间的通信
sem_t rwsem;

// 记录登录状态是否成功
atomic_bool g_isLoginSuccess{false};

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
// 主聊天页面程序
void mainChatPage(int clientfd);
// 显示当前登录成功用户的基本信息
void showCurrentUserInfo();

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "Usage: ./ChatClient <ip> <port>" << endl;
        return -1;
    }
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 1.创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0)
    {
        cout << "socket error" << endl;
        return -1;
    }
    // 2.连接服务器
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        cout << "connect error" << endl;
        close(clientfd);
        return -1;
    }

    // 初始化读写线程的信号量
    sem_init(&rwsem, 0, 0);

    // 连接服务器成功 启动接收子线程
    thread readTask(readTaskHandler, clientfd);
    readTask.detach(); // 分离线程

    // main线程用于接收用户输入，负责发送数据
    for (;;)
    {
        cout << "======================ChatClient======================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "======================================================" << endl;
        int choice = 0;
        cin >> choice;
        cin.get(); // 清除换行符
        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "Please enter your userid: ";
            cin >> id;
            cin.get(); // 清除换行符
            cout << "Please enter your password: ";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoginSuccess = false;

            // 发送登录请求
            int len = send(clientfd, request.c_str(), request.size(), 0);
            if (len < 0)
            {
                cout << "send error" << endl;
            }

            // 等待信号量，由子线程处理完登录的响应消息后，通知这里
            sem_wait(&rwsem);

            if (g_isLoginSuccess)
            {
                // 进入主聊天页面
                isMainMennuRunning = true;
                mainChatPage(clientfd);
            }
        }
        break;
        case 2:
        {
            // 注册
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "Please enter your name: ";
            cin.getline(name, 50);
            cout << "Please enter your password: ";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();
            // 发送注册请求
            int len = send(clientfd, request.c_str(), request.size(), 0);
            if (len == -1)
            {
                cout << "send error" << endl;
            }
            // 等待信号量，由子线程处理完登录的响应消息后，通知这里
            sem_wait(&rwsem);
        }
        break;
        case 3: // quit业务
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        default:
            cout << "Invalid choice, please try again." << endl;
            break;
        }
    }
    return 0;
}

// 显示当前登录成功用户的基本信息
void showCurrentUserInfo()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriends.empty())
    {
        for (User &user : g_currentUserFriends)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroups.empty())
    {
        for (Group &group : g_currentUserGroups)
        {
            cout << group.getId() << " " << group.getGroupName() << " " << group.getGroupDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// 处理登录响应逻辑
void doLoginResponse(json &response)
{
    if (response["errno"].get<int>() == 0) // 登录成功
    {
        g_isLoginSuccess = true;

        // 记录当前用户的id和name
        g_currentUser.setId(response["id"].get<int>());
        g_currentUser.setName(response["name"].get<string>());
        // 记录当前用户的好友信息
        if (response.contains("friends"))
        {
            // 初始化
            g_currentUserFriends.clear();
            for (auto &item : response["friends"])
            {
                string friend_str = item.get<string>();
                // 将字符串解析为json对象
                json js = json::parse(friend_str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"].get<string>());
                user.setState(js["state"].get<string>());
                g_currentUserFriends.push_back(user);
            }
        }
        // 记录当前用户的群组信息
        if (response.contains("groups"))
        {
            // 初始化
            g_currentUserGroups.clear();
            for (auto &item : response["groups"])
            {
                Group group;
                string item_str = item.get<string>();
                json groupjs = json::parse(item_str);
                group.setId(groupjs["id"].get<int>());
                group.setGroupName(groupjs["groupname"].get<string>());
                group.setGroupDesc(groupjs["groupdesc"].get<string>());

                // 记录群组成员信息
                for (auto &user : groupjs["users"])
                {
                    GroupUser groupUser;
                    string user_str = user.get<string>();
                    json userjs = json::parse(user_str);
                    groupUser.setId(userjs["id"].get<int>());
                    groupUser.setName(userjs["name"].get<string>());
                    groupUser.setState(userjs["state"].get<string>());
                    groupUser.setRole(userjs["role"].get<string>());
                    group.getUsers().push_back(groupUser);
                }
                // 将群组信息添加到当前用户的群组列表中
                g_currentUserGroups.push_back(group);
            }
        }

        showCurrentUserInfo();

        // 显示当前用户的离线信息 个人聊天信息或者群组消息
        if (response.contains("offlinemsg"))
        {
            vector<string> vec = response["offlinemsg"];
            for (string &msg : vec)
            {
                // 接收chatserver发送的消息，反序列化生成json对象
                json js = json::parse(msg);

                if (js["msgid"].get<int>() == ONE_CHAT_MSG)
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"] << endl;
                    continue;
                }
                else if (js["msgid"].get<int>() == GROUP_CHAT_MSG)
                {
                    cout << "群消息[ " << js["groupid"] << "] " << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"] << endl;
                    continue;
                }
            }
        }
    }
}

// 处理注册响应逻辑
void doRegResponse(json &response)
{
    // 注册成功，接收服务器返回的注册结果
    if (response["errno"].get<int>() == 0) // 注册成功
    {
        cout << "register success, your userid is: " << response["id"] << endl;
    }
    else // 注册失败
    {
        cout << "register failed, please try again." << endl;
    }
}

// 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int ret = recv(clientfd, buffer, 1024, 0);
        if (ret > 0)
        {
            // 接收chatserver发送的消息，反序列化生成json对象
            json response = json::parse(buffer);
            if (response["msgid"].get<int>() == ONE_CHAT_MSG)
            {
                cout << response["time"].get<string>() << " [" << response["id"] << "]" << response["name"] << " said: " << response["msg"] << endl;
                continue;
            }
            if (response["msgid"].get<int>() == GROUP_CHAT_MSG)
            {
                cout << "群消息[ " << response["groupid"] << "] " << response["time"].get<string>() << " [" << response["id"] << "]" << response["name"] << " said: " << response["msg"] << endl;
                continue;
            }
            if (response["msgid"].get<int>() == LOGIN_MSG_ACK)
            {
                // 处理登录响应的业务逻辑
                doLoginResponse(response);
                sem_post(&rwsem);
                continue;
            }
            if (response["msgid"].get<int>() == REG_MSG_ACK)
            {
                // 处理注册响应的业务逻辑
                doRegResponse(response);
                sem_post(&rwsem);
                continue;
            }
        }
        else
        {
            cout << "recv error" << endl;
            close(clientfd);
            exit(-1);
        }
    }
}
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    tm *local_time = localtime(&now_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
    return string(buffer);
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

unordered_map<string, function<void(int, string)>> commandhandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 主聊天页面程序
void mainChatPage(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMennuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuffer(buffer);
        string command(buffer);

        int idx = commandbuffer.find(':');
        if (idx == -1)
        {
            command = commandbuffer;
        }
        else
        {
            command = commandbuffer.substr(0, idx);
        }
        auto it = commandhandlerMap.find(command);
        if (it == commandhandlerMap.end())
        {
            cout << "Invalid command, please try again." << endl;
            continue;
        }
        it->second(clientfd, commandbuffer.substr(idx + 1, commandbuffer.size() - idx - 1));
    }
}

// "help" command handler
void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size(), 0);
    if (len < 0)
    {
        cout << "send error" << endl;
    }
}

// str——————————friendid:message
void chat(int clientfd, string str)
{
    int idx = str.find(':');
    if (idx == -1)
    {
        cout << "Invalid command, please try again." << endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx - 1);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string request = js.dump();

    int len = send(clientfd, request.c_str(), request.size(), 0);
    if (len < 0)
    {
        cout << "send error" << endl;
    }
}

// "creategroup" command handler  creategroup:groupname:groupdesc  str->groupname:groupdesc
void creategroup(int clientfd, string str)
{
    int idx = str.find(':');
    if (idx == -1)
    {
        cout << "Invalid command, please try again." << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx - 1);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size(), 0);
    if (len < 0)
    {
        cout << "send creategroup error" << endl;
    }
}
// "addgroup" command handler  addgroup:groupid  str->groupid
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), request.size(), 0);
    if (len < 0)
    {
        cout << "send addgroup error" << endl;
    }
}
// "groupchat" command handler  groupchat:groupid:message  str->groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(':');
    if (idx == -1)
    {
        cout << "Invalid command, please try again." << endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx - 1);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string request = js.dump();

    int len = send(clientfd, request.c_str(), request.size(), 0);
    if (len < 0)
    {
        cout << "send groupchat message error" << endl;
    }
}
// "loginout" command handler
void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len < 0)
    {
        cout << "send loginout message error" << endl;
    }
    else
    {
        isMainMennuRunning = false;
    }
}