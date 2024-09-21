#include "inc/include.h"

namespace xlog
{
    void __msg(const char *fmt, ...)
    {
        time_t tm_curr = time(0);

        va_list vl;
        va_start(vl, fmt);
        printf("MSG    ");
        vprintf(fmt, vl);
        printf("\n");
        va_end(vl);
    }

    void __error(const char *fmt, ...)
    {
        time_t tm_curr = time(0);

        va_list vl;
        va_start(vl, fmt);
        printf("ERROR    ");
        vprintf(fmt, vl);
        printf("\n");
        va_end(vl);
    }
}
