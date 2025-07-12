#pragma once
#include "InetAddress.h"
#include "Buffer.h"
#include "Socket.h"
#include <memory>
#include <functional>

#include <any>
namespace mylib
{
    class TcpConnection;
    class Timestamp;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                               Buffer *buf,
                                               Timestamp)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
    using ReadableCallback = std::function<void(const TcpConnectionPtr &)>;
    using WriteCallback=std::function<void(const TcpConnectionPtr&)>;


    class EventLoop;
    class Socket;
    class Channel;
    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {

    public:
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);

        EventLoop *getLoop() const { return loop_; }
        bool connected() { return state_ == kConnected; }
        const std::string &name() const { return name_; }
        const InetAddress &localAddress() { return localAddr_; }
        const InetAddress &peerAddress() { return peerAddr_; }

        void connectEstablished();

        void setReadableCallback(const ReadableCallback &cb)
        {
            if (cb)
                is_setReadableCallback = true;
            else
                is_setReadableCallback = false;
            readableCallback_ = cb;
        }

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }
        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }
        void setCloseCallback(const CloseCallback &cb)
        {
            closeCallback_ = cb;
        }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            writeCompleteCallback_ = cb;
        }
        void setWriteCallback(const WriteCallback&cb)
        {
            writeCallback_=cb;
        }

        void connectDestroyed();
        void shutdown();
        void send(const std::string &message);
        void setTcpNodelay(bool on);

        std::string user_id;

        Socket *socket() { return socket_.get(); }

        void setContext(const std::any &context) { context_ = context; }
        const std::any &getContext() const { return context_; }
        std::any *getMutableContext() { return &context_; }

        std::unique_ptr<Channel> channel_;

    private:
        enum StateE
        {
            kConnecting,
            kConnected,
            kDisconnecting,
            kDisconnected,
        };
        void setState(StateE s) { state_ = s; }

        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();
        void sendInLoop(const std::string &message);
        void shutdownInLoop();

        EventLoop *loop_;
        std::string name_;
        StateE state_;

        InetAddress localAddr_;
        InetAddress peerAddr_;

        std::unique_ptr<Socket> socket_;

        MessageCallback messageCallback_;
        ConnectionCallback connectionCallback_;
        CloseCallback closeCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        ReadableCallback readableCallback_;
        WriteCallback writeCallback_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;

        std::any context_;

        bool is_setReadableCallback = false;
    };
};
