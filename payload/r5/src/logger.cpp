#include "inc/include.h"

namespace logger
{
    void log2file(const char *contents)
    {
    #if defined(_RI_ENABLE_LOGGING) || defined(DECLARE_AS_DEBUG_BUILD) || defined(_RI_ENABLE_LOGGING_MINIMAL)
        auto fp = stl::Xfopen(_XS("/home/drof/Documents/log.txt"), _XS("a"));
        if (fp)
        {
            stl::Xfputs(contents, fp);
            stl::Xfputs("\n", fp);
            stl::Xfclose(fp);
        }
    #else
        __asm nop;
    #endif
    }
}
