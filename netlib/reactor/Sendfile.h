#pragma once
#include <memory>
#include <stdio.h>
#include <TcpConnection.h>

#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
using namespace mylib;
struct SendFileContext
{
    int fileFd;
    off_t offset;
    off_t totalSize;
    SendFileContext(int fd, off_t off, off_t size) : fileFd(fd),
                                                     offset(off),
                                                     totalSize(size) {}
};

void sendFileChunk(const TcpConnectionPtr &conn)
{
    auto ctx_ptr = std::any_cast<std::shared_ptr<SendFileContext>>(conn->getMutableContext());
    auto &ctx = *ctx_ptr;
    while (ctx->offset < ctx->totalSize)
    {
        off_t remain = ctx->totalSize - ctx->offset;
        ssize_t sent = ::sendfile(conn->socket()->fd(), ctx->fileFd, &ctx->offset, remain);
        if (sent <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                conn->setWriteCompleteCallback([ctx](const TcpConnectionPtr &conn)
                                               { sendFileChunk(conn); });
                return;
            }
            else if (errno == EINTR)
                continue;
            else
            {
                perror("sendfile");
                ::close(ctx->fileFd);
                conn->setWriteCompleteCallback(nullptr);
                conn->setContext(std::any());
                conn->shutdown();
                return;
            }
        }
    }

    ::close(ctx->fileFd);
    conn->setWriteCompleteCallback(nullptr);
    conn->setContext(std::any());
    conn->shutdown();
}

void sendFile(const TcpConnectionPtr &conn, int fileFd, off_t offset, off_t fileSize)
{
    auto ctx = std::make_shared<SendFileContext>(fileFd, offset, fileSize);
    conn->setContext(ctx);
    sendFileChunk(conn);
}
