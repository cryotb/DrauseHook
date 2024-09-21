#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef u64 uptr;
typedef void* ptr;

typedef signed char n8;
typedef signed short n16;
typedef signed long n32;
typedef signed long long n64;

#define OBFUS_FUNC __attribute__((__annotate__((""))))
#define OBFUS_FUNC_EX __attribute__((__annotate__((""))))
#define OBFUS_FUNC_SIG __attribute__((__annotate__((""))))
#define OBFUS_FUNC_S __attribute__((__annotate__((""))))

#define LLVMT(...) __attribute__(( __VA_ARGS__ ))
#define LLVM_INLINE inline
#define LLVM_NOINLINE __attribute__((noinline))
#define LLVM_NOOPT [[clang::optnone]]
#define LLVMOBF_INLINE_FP() __annotate__(("inline-fps"))
#define LLVMOBF_CALLS() __annotate__(("fneobfcll"))
#define LLVMOBF_SUB(loop) __annotate__(("sub")), __annotate__(("sub_loop=" #loop))
#define LLVMOBF_BCF(prob, loop) __annotate__(("bcf")), __annotate__(("bcf_prob=" #prob)), __annotate__(("bcf_loop=" #loop))
#define LLVMOBF_SPLIT(num) __annotate__(("split")), __annotate__(("split_num=" #num))

#if defined(PROD)
#define LLVMOBF_COMBO_STANDARD LLVMOBF_SUB(2), LLVMOBF_BCF(20, 2), LLVMOBF_SPLIT(2)
#else
#define LLVMOBF_COMBO_STANDARD
#endif

#define OBFUS_FUNC_DISABLE __attribute__((__annotate__(("nobcf")), __annotate__(("nofla")),  \
__annotate__(("nosub")), __annotate__(("nosplit"))))

#define BASE_OF(X) ((u64)X)
#define PTR_OF(X) ((void*)X)

#define FWD_DECLARE_CLASS(X) class X
#define DECLARE_OFFSET(NAME, OFF) inline constexpr ptrdiff_t NAME = OFF

#ifndef __FLTUSED__
#define __FLTUSED__
extern "C" __declspec(selectany) int _fltused = 1;
#endif

typedef u8 uint8;
typedef u16 uint16;
typedef u32 uint32;
typedef u64 uint64;

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

using char256 = char[256];

template<class T> __forceinline T __ROL__(T value, int count)
{
    const uint nbits = sizeof(T) * 8;

    if (count > 0)
    {
        count %= nbits;
        T high = value >> (nbits - count);
        if (T(-1) < 0) // signed value
            high &= ~((T(-1) << count));
        value <<= count;
        value |= high;
    }
    else
    {
        count = -count % nbits;
        T low = value << (nbits - count);
        value >>= count;
        value |= low;
    }
    return value;
}

__forceinline uint8  __ROL1__(uint8  value, int count) { return __ROL__((uint8)value, count); }
__forceinline uint16 __ROL2__(uint16 value, int count) { return __ROL__((uint16)value, count); }
__forceinline uint32 __ROL4__(uint32 value, int count) { return __ROL__((uint32)value, count); }
__forceinline uint64 __ROL8__(uint64 value, int count) { return __ROL__((uint64)value, count); }
__forceinline uint8  __ROR1__(uint8  value, int count) { return __ROL__((uint8)value, -count); }
__forceinline uint16 __ROR2__(uint16 value, int count) { return __ROL__((uint16)value, -count); }
__forceinline uint32 __ROR4__(uint32 value, int count) { return __ROL__((uint32)value, -count); }
__forceinline uint64 __ROR8__(uint64 value, int count) { return __ROL__((uint64)value, -count); }

#define encrypt_constant(value) (__ROL8__((value), 2) ^ BUILD_SEED)
LLVM_NOOPT extern u64 decrypt_constant(u64 value);
#define _encoded_const(value) decrypt_constant( encrypt_constant(value) )

#define _scex(FUNC, ...) invoke(FUNC, __VA_ARGS__)
#define _scex_wrt(RET, FUNC, ...) invoke_withrettype<RET>(FUNC, __VA_ARGS__)

#define DATA_FIELD(name, type, offset) __attribute__((always_inline))  inline type& name() { return *(type*)(m_base + offset); }
#define DATA_FIELD_NR(NAME, INST, TYPE, OFFSET) __attribute__((always_inline)) inline TYPE* NAME() { return (TYPE*)( (uptr)INST + OFFSET); }

#define DATA_FIELD_DIRECT(name, type, offset) __attribute__((always_inline))  inline type& name() { return *(type*)(BASE_OF(this) + offset); }

#define Xsprintf stl::fps::sprintf

#define MAKE_LOG_BUF(NAME, LEN) \
	char NAME[LEN];             \
	memset(NAME, 0, sizeof(NAME));

#if defined(DECLARE_AS_DEBUG_BUILD)
#define IS_DEBUG_BUILD() true
#else
#define IS_DEBUG_BUILD() false
#endif

#if defined(_RI_ENABLE_LOGGING) || defined(DECLARE_AS_DEBUG_BUILD)
#define LOGGING_IS_ENABLED 1
#else
#define LOGGING_IS_ENABLED 0
#endif

#define Plat_FloatTime() g.gvars->Curtime 
#define Plat_FloatTick() g.gvars->TickCount

#define MmIsAddressValid(address) \
    (((uintptr_t)(address) >= 0x0000000000000000) && ((uintptr_t)(address) <= 0x00007fffffffffff))


#define QUOTE(str) #str
#define STRINGIFIED_DEF(str) QUOTE(str)
