#pragma once

#include <utility>

#define WINAPI_INLINE LLVM_NOOPT __attribute((noinline)) inline
#define SYSV_API_INLINE LLVM_NOOPT __attribute((noinline, sysv_abi)) inline

namespace stl
{
    inline uptr baseof_libc = 0;
    inline uptr baseof_libm = 0;

    bool init(uptr libc_base, uptr libm_base);

    namespace prototypes
    {
        typedef int (*fn_sprintf)(char *, const char *, ...) __attribute__((sysv_abi));
        typedef u32 (*fn_sleep)(u32) __attribute__((sysv_abi));
    }

    namespace fps
    {
        inline prototypes::fn_sprintf sprintf = nullptr;
        inline prototypes::fn_sleep sleep = nullptr;
    }

    SYSV_API_INLINE auto Xfopen(const char *path, const char *mode)
    {
        using fn = void *(*)(const char *, const char *);
        return reinterpret_cast<fn>(baseof_libc + gctx->linuxcrt_cpp.fopen)(path, mode);
    }

    SYSV_API_INLINE auto Xfputs(const char *buf, void *fp)
    {
        using fn = int (*)(const char *, void *);
        return reinterpret_cast<fn>(baseof_libc + gctx->linuxcrt_cpp.fputs)(buf, fp);
    }

    SYSV_API_INLINE auto Xfclose(void *fp)
    {
        using fn = int (*)(void *);
        return reinterpret_cast<fn>(baseof_libc + gctx->linuxcrt_cpp.fclose)(fp);
    }

    /*
        Math
    */
    SYSV_API_INLINE auto Xacosf(float val)
    {
        using fn = float (*)(float);
        return reinterpret_cast<fn>(baseof_libm + gctx->linuxcrt_math.acosf)(val);
    }

    SYSV_API_INLINE auto Xatan2f(float y, float x)
    {
        using fn = float (*)(float, float);
        return reinterpret_cast<fn>(baseof_libm + gctx->linuxcrt_math.atan2f)(y, x);
    }

    SYSV_API_INLINE auto Xfabsf(float val)
    {
        using fn = float (*)(float);
        return reinterpret_cast<fn>(baseof_libm + gctx->linuxcrt_math.fabsf)(val);
    }

    SYSV_API_INLINE auto Xsinf(__m128 xx)
    {
        using fn = float (*)(__m128);
        return reinterpret_cast<fn>(baseof_libm + gctx->linuxcrt_math.sinf)(xx);
    }

    SYSV_API_INLINE auto Xcosf(__m128 xx)
    {
        using fn = float (*)(__m128);
        return reinterpret_cast<fn>(baseof_libm + gctx->linuxcrt_math.cosf)(xx);
    }

    inline auto file_exists(const char *path)
    {
        if (auto handle = Xfopen(path, "r"))
        {
            Xfclose(handle);
            return true;
        }

        return false;
    }
    
    LLVM_NOOPT extern void *global_mm_alloc(size_t len);
    LLVM_NOOPT extern void global_mm_free(void *p);

    template <typename T, typename... Args>
    T *alloc_type(Args... args)
    {
        void *memory = global_mm_alloc(sizeof(T));
        if (memory == nullptr)
        {
            return nullptr;
        }

        T *object = new (memory) T(args...);
        return object;
    }

    inline uintptr_t search(const uint8_t *data, const uint8_t *end, const uint8_t *needle, size_t needle_len)
    {
        if (needle_len > 0)
        {
            for (auto *curr = data; curr < end; curr++)
            {
                if (curr[0] == needle[0])
                {
                    if (needle_len == 1)
                        return (uintptr_t)curr;

                    auto complen = needle_len - 1;
                    auto comppos = &curr[1];
                    auto needlepos = &needle[1];
                    size_t num_matching = 0;

                    for (size_t i = 0; i < complen; i++)
                    {
                        if (comppos[i] == needlepos[i])
                        {
                            num_matching++;
                            continue;
                        }

                        break;
                    }

                    if (num_matching == complen)
                        return (uintptr_t)curr;
                }
            }
        }

        return 0;
    }
}
