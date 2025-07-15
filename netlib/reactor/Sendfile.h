#pragma once
#include <memory>
#include <stdio.h>
#include <TcpConnection.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
using namespace mylib;
namespace fs = std::filesystem;

struct FileContext
{
    int fileFd;
    int fileId;
    off_t offset;
    off_t totalSize;
    std::string file_name;
    ssize_t written = 0;
    int pipefd[2];
    long long count = 0;
    FileContext(int fd, int id, off_t off, off_t size) : fileFd(fd),
                                                         fileId(id),
                                                         offset(off),
                                                         totalSize(size) {}
};

void sendFileChunk(const TcpConnectionPtr &conn)
{
    auto ctx = std::any_cast<std::shared_ptr<FileContext>>(conn->getContext());
    while (ctx->offset < ctx->totalSize)
    {
        off_t remain = ctx->totalSize - ctx->offset;
        std::cout << ctx->count++ << std::endl;
        ssize_t sent = ::sendfile(conn->socket()->fd(), ctx->fileFd, &ctx->offset, remain);
        if (sent <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                conn->setWriteCallback([ctx](const TcpConnectionPtr &conn)
                                       { sendFileChunk(conn); });
                conn->channel_->enableWriting();

                return;
            }
            else if (errno == EINTR)
                continue;
            else
            {
                perror("sendfile");
                ::close(ctx->fileFd);
                conn->setWriteCallback(nullptr);
                conn->setContext(std::any());
                conn->channel_->disableWriting();
                return;
            }
        }
    }
    LOG_INFO("文件[%d]发送完毕", ctx->fileId);
    ::close(ctx->fileFd);
    conn->setWriteCallback(nullptr);
    conn->setContext(std::any());
    conn->channel_->disableWriting();
    conn->shutdown();
}

bool isConnected(const TcpConnectionPtr &conn)
{
    char buf[1];
    ssize_t n = ::recv(conn->socket()->fd(), buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (n == 0)
        return false;
    else
        return true;
}

void recvFileData(const TcpConnectionPtr &conn)
{

    auto ctx = std::any_cast<std::shared_ptr<FileContext>>(conn->getContext());
    while (1)
    {
        ssize_t n = splice(conn->socket()->fd(), nullptr, ctx->pipefd[1], nullptr, 65535, SPLICE_F_MOVE);
        std::cout << n << std::endl;
        if (n > 0)
        {
            std::cout << n << std::endl;
            ssize_t written = splice(ctx->pipefd[0], nullptr, ctx->fileFd, nullptr, n, SPLICE_F_MOVE);
            ctx->written += written;
            if (ctx->written >= ctx->totalSize)
            {
                ::close(ctx->fileFd);
                ::close(ctx->pipefd[0]);
                ::close(ctx->pipefd[1]);
                // conn->setContext(std::any());
                //  conn->channel_->disableAll();
                // conn->shutdown();
            }
        }
        else if (errno == EAGAIN)
        {
            break;
        }
        // 尝试读取少量数据检查连接状态
        if (!isConnected(conn))
        {
            conn->setContext(std::any());
            conn->forceClose();
            break;
        }
    }
}

void sendFile(const TcpConnectionPtr &conn, int fileFd, off_t offset, off_t fileSize, int fileId = 0)
{
    auto ctx = std::make_shared<FileContext>(fileFd, fileId, offset, fileSize);
    conn->setContext(ctx);
    sendFileChunk(conn);
}

std::string extractFilename(const std::string &filePath)
{
    fs::path p(filePath.c_str());
    return p.filename().string();
}

off_t getFileSize(int fd)
{
    struct stat file_info;
    ::fstat(fd, &file_info);
    return file_info.st_size;
}