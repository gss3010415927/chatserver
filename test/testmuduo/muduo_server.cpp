/*
muduo网络库提供了两个主要的类
TcpServer
TcpClient
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
// 基于muduo库的TcpServer类实现一个简单的服务器
// 1. 创建一个TcpServer对象
// 2. 创建EventLoop事件循环对象
// 3. 创建ChatServer构造函数
// 4. 注册处理连接的回调函数和处理读写事件的回调函数
// 5. 设置合适的线程池数量， muduo会自动分配IO线程和工作线程
class ChatServer
{
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接和断开回调
        _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置服务器的线程池 1个I/O线程 3个工作线程
        _server.setThreadNum(4);
    }

    // 开启事件循环
    void start()
    {
        // 开启事件循环
        _server.start();
    }

private:
    // 用户连接和断开回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            // 用户连接成功
            cout << conn->peerAddress().toIpPort() << "->"
                 << conn->localAddress().toIpPort() << "state on" << endl;
        }
        else
        {
            // 用户断开连接
            cout << conn->peerAddress().toIpPort() << "->"
                 << conn->localAddress().toIpPort() << "state off" << endl;
                 conn->shutdown(); //关闭连接
        }
    }
    // 处理用户读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data: " << buf << "time: " << time.toString() << endl;
        conn->send(buf); // 回发数据
    }
    TcpServer _server;
    EventLoop *_loop;
};

int main()
{
    //创建事件循环对象
    EventLoop loop;
    InetAddress addr("127.0.0.1",8000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    // 事件循环
    loop.loop();    //epoll_wait 以阻塞方式等待新用户连接或者以连接用户的读写事件
    return 0;
}