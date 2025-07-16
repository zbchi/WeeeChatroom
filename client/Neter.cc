#include "Neter.h"
#include "Client.h"

void Neter::start()
{
    loop_ = loopThread_.startLoop(); // 开新的线程接收消息
    Tcpclient_ = std::make_unique<mylib::TcpClient>(loop_, serverAddr_);

    Tcpclient_->setConnectionCallback([this](const TcpConnectionPtr &conn)
                                      { if(conn->connected())
                                {
                                    std::lock_guard<std::mutex> lock(connMutex_);
                                    conn_=conn;
                                }
                                else
                                std::cout<<"disconnected!!-----!!!!!!----"<<std::endl;
                                connCond_.notify_one(); });
    Tcpclient_->setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                   { this->onMessage(conn, buf, time); });
    Tcpclient_->connect();
    loop_->runEvery(10.0, [this]()
                    {   json js; 
                        sendJson(js); }); // 发送心跳包，发送null

    // 阻塞等待recv线程连接
    {
        std::unique_lock<std::mutex> lock(connMutex_);
        connCond_.wait(lock, [this]
                       { return conn_ != nullptr; });
    }

    // 主线程入口
}

void Neter::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    while (buf->readableBytes() >= 4)
    {
        const void *data = buf->peek();
        int lenNetOrder;
        memcpy(&lenNetOrder, data, sizeof(lenNetOrder));
        int len = ntohl(lenNetOrder);
        if (buf->readableBytes() < 4 + len)
            break;
        buf->retrieve(4);

        std::string jsonStr(buf->peek(), len);
        buf->retrieve(len);
        // std::cout << jsonStr << std::endl;

        loop_->runInLoop([this, conn, jsonStr]()
                         { client_->handleJson(conn, jsonStr); });
    }
}

void Neter::sendJson(json &js)
{
    std::string jsonStr = js.dump();
    int len = static_cast<int>(jsonStr.size());
    int beLen = htonl(len);
    std::string msg;
    msg.append(reinterpret_cast<const char *>(&beLen), sizeof(beLen));
    msg.append(jsonStr);
    conn_->send(msg);
}