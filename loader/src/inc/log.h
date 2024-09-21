#pragma once

namespace xlog
{
    void __msg(const char* fmt, ...);
    void __error(const char* fmt, ...);
    void __safeLog_msg(const char* fmt, ...);
}

#define msg(X, ...) xlog::__msg(_XS(X), ##__VA_ARGS__)
#define error(X, ...) xlog::__error(_XS(X), ##__VA_ARGS__)

// TODO: implement!
#define dbg(X, ...) while(0){ }

#if defined(R5I_PROD)
#define safeLog_msg(X, ...) CloudLog_Msg(_XS(X), ##__VA_ARGS__)
#else
#define safeLog_msg msg
#endif
