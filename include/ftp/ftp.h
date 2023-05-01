#pragma once

#define ARG_BUF_SIZE 0x80
#define DATA_BUF_SIZE 1024
#define PATH_BUF_SIZE 4096

enum FTP_DATA_MODE {
    DATA_MODE_ASCII,
    DATA_MODE_IMAGE
};
enum FTP_XFER_MODE {
    XFER_MODE_PORT,
    XFER_MODE_PASV
};

struct FTPSession;
struct FTPCommand {
    char* name;
    int (*handler)(FTPSession* session, const char* args);
};

namespace FTP {

    void start();

} // namespace FTP