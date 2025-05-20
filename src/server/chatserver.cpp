#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace std::placeholders;
using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : _server(loop, listenAddr, nameArg),
                                                _loop(loop)
{
    // 设置连接回调函数
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    // 设置读写事件回调函数
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数量
    _server.setThreadNum(4);
}
// 开启事件循环
void ChatServer::start()
{
    _server.start();
}
// 用户连接和断开回调
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
// 读写事件回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 解析json数据
    json js = json::parse(buf);
    // 达到的目的：完全解耦网络模块代码和业务模块代码
    // 通过message_id获取业务处理器，并执行相应的业务操作
    ChatService::instance()->getHandler(js["msgid"].get<int>())(conn, js, time);
}
