#pragma once

namespace logger
{
    void log2file(const char* contents);
}

__attribute((noinline)) inline void msg(const char* contents)
{
    logger::log2file(contents);
}

#define LMsg(buflen, fmt, ...) \
    do { \
        char log_buf[buflen]; \
        Xsprintf(log_buf, fmt, ##__VA_ARGS__); \
        msg(log_buf); \
    } while (0)

#define LMsg_Short(fmt, ...) LMsg(256, fmt, __VA_ARGS__)
