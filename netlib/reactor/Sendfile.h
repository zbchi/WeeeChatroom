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

const int SEND_FILE_SIZE = 1024 * 128;
const int SPLICE_SIZE = 65535;

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
    
    std::string sender_id;
    std::string peer_id;
    std::string file_path;

    bool is_group;
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
        ssize_t sent = ::sendfile(conn->socket()->fd(), ctx->fileFd, &ctx->offset, SEND_FILE_SIZE);
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
        // 尝试读取少量数据检查连接状态
        if (!isConnected(conn))
        {
            // conn->setContext(std::any());
            LOG_DEBUG("断开连接");
            conn->forceClose();
            break;
        }
        ssize_t n = splice(conn->socket()->fd(), nullptr, ctx->pipefd[1], nullptr, SPLICE_SIZE, SPLICE_F_MOVE);
        if (n > 0)
        {
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