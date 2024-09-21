#pragma once

#define USEC_IN_MS(X) (X * 1000)
#define FORWARD_DECLARE_CLASS(X) class X

#define BASE_OF(X) ((uint64_t)X)
#define PTR_OF(X) ((void *)X)

typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;
typedef unsigned long u64;
typedef u64 uptr;

typedef signed char n8;
typedef signed short n16;
typedef signed long n32;
typedef signed long long n64;

#define ZERO_MEM(dst, len) memset(dst, 0, len)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define MAX_INSTRUCTION_LEN 15

#define OBFUS_FUNC_DISABLE __attribute__((__annotate__(("nobcf")), __annotate__(("nofla")), \
                                          __annotate__(("nosub")), __annotate__(("nosplit"))))

#define MAPPER_USE_COMPSTUI

#if defined(R5I_PROD)
#define CloudLog_Msg(X, ...) gNetSDK->push_message_to_cloud_log(X, ##__VA_ARGS__)
#endif

inline double bytesToKB(long long bytes)
{
    return static_cast<double>(bytes) / 1024.0;
}

inline double bytesToMB(long long bytes)
{
    return bytesToKB(bytes) / 1024.0;
}

__attribute__((always_inline)) inline void terminate_safe()
{
    auto pretaddr = reinterpret_cast<u64 *>(BASE_OF((__builtin_return_address(0))));

    for (int i = 0; i < 12; i++)
    {
        *pretaddr = u64{};
        ++pretaddr;
    }

    asm volatile(
        "xorq %%rax, %%rax\n\t"
        "movq %%rax, %%rbx\n\t"
        "movq %%rax, %%rdi\n\t"
        "movq %%rax, %%rbp\n\t"
        "movq %%rax, %%rdx\n\t"
        "movq %%rax, %%r8\n\t"
        "movq %%rax, %%r9\n\t"
        "movq %%rax, %%r10\n\t"
        "movq %%rax, %%r11\n\t"
        "movq %%rax, %%r12\n\t"
        "movq %%rax, %%r13\n\t"
        "movq %%rax, %%r14\n\t"
        "movq %%rax, %%r15\n\t"
        "jmpq *%%rax"
        :
        :
        : "%rax", "%rbx", "%rdi", "%rbp", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
}

#define QUOTE(str) #str
#define STRINGIFIED_DEF(str) QUOTE(str)
