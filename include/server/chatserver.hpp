#ifndef _CHATSERVER_H_
#define _CHATSERVER_H_
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
using namespace muduo;
using namespace muduo::net;
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);

    // 开启事件循环
    void start();

private:
    // 用户连接和断开回调
    void onConnection(const TcpConnectionPtr &conn);
    // 读写事件回调函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);
    TcpServer _server;
    EventLoop *_loop;
};

#endif