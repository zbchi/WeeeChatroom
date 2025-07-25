#pragma once
#include "Handler.h"
#include "base.h"
#include "EventLoop.h"
#include "TcpServer.h"
using namespace mylib;
class Service;

class FtpServer
{
public:
    FtpServer();
    void start();
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    void onConnection(const TcpConnectionPtr &conn);

    EventLoop loop_;
    InetAddress listenAddr_;
    TcpServer tcpServer_;

private:
    std::string makeFilePath(const std::string &file_id);
    std::string generateUUID();
};

class FileUploader : public Handler
{
public:
    FileUploader(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};

class FileLister : public Handler
{
public:
    FileLister(Service *service) : service_(service) {}
    void handle(const TcpConnectionPtr &conn, json &js, Timestamp time);

private:
    Service *service_;
};