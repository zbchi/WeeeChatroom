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

#pragma pack(1)
struct FileHeader
{
    uint8_t type;
    uint64_t file_size;
    char file_id[32];
    char file_name[64];
};
#pragma pack()

struct FileContext
{
    int fileFd;
    int fileId;
    off_t offset;
    off_t totalSize;
    std::string file_name;
    ssize_t written = 0;
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
        ssize_t sent = ::sendfile(conn->socket()->fd(), ctx->fileFd, &ctx->offset, remain);
        std::cout << "---------------------------------" << std::endl;
        if (sent <= 0)
        {
            std::cout << "sent<0sent<0sent<0sent<0sent<0" << std::endl;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "EAGAINEAGINEAGAINEAGINEAGIN" << std::endl;
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

    ::close(ctx->fileFd);
    conn->setWriteCallback(nullptr);
    conn->setContext(std::any());
    conn->channel_->disableWriting();
}

void sendFile(const TcpConnectionPtr &conn, int fileFd, off_t offset, off_t fileSize)
{
    auto ctx = std::make_shared<FileContext>(fileFd, 0, offset, fileSize);
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