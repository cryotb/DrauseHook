#pragma once

namespace stl
{
    using fn_mem_alloc = void*(__fastcall*)(size_t);
    using fn_mem_free = void(__fastcall*)(void*);
}

LLVM_NOOPT inline size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s != '\0')
    {
        ++len;
        ++s;
    }
    return len;
}

LLVM_NOOPT inline uint32_t string_hash_fnv1a(const char *str)
{
    const uint32_t prime = 0x1000193;
    const uint32_t offset = 0x811c9dc5;
    uint32_t hash = offset;

    for (const char *p = str; *p; p++)
    {
        hash ^= *p;
        hash *= prime;
    }

    return hash;
}

LLVM_NOOPT inline int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);
    return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

LLVM_NOOPT inline int strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return (0);
    do
    {
        if (*s1 != *s2++)
            return (*(unsigned char *)s1 - *(unsigned char *)--s2);
        if (*s1++ == 0)
            break;
    } while (--n != 0);

    return (0);
}

LLVM_NOOPT inline char *strstr(const char *s, const char *find)
{
    char c, sc;
    size_t len;
    if ((c = *find++) != 0)
    {
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while (sc != c);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

LLVM_NOOPT inline int wcscmp(const wchar_t *s1, const wchar_t *s2)
{
    while (*s1 && *s2 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return (*s1 - *s2);
}

LLVM_NOOPT inline char *strcpy(char *d, const char *s)
{
    char *saved = d;

    while (*s)
    {
        *d++ = *s++;
    }

    *d = 0;

    return saved;
}

LLVM_NOOPT inline char* find_substr(char* data, const char* needle)
{
	if (!data || !needle)
		return nullptr;

	size_t data_len = strlen(data);
	size_t needle_len = strlen(needle);

	for (size_t i = 0; i <= data_len - needle_len; i++)
	{
		size_t j;
		for (j = 0; j < needle_len; j++)
		{
			if (data[i + j] != needle[j])
				break;
		}

		if (j == needle_len)
			return &data[i];
	}

	return nullptr;
}

void __guaranteed_stosb(void *dest, unsigned char value, size_t count);
void __guaranteed_movsb(void *dest, const void *src, size_t count);

#define memset(dst, val, len) __guaranteed_stosb((unsigned char *)dst, val, len)
#define memcpy(dst, src, len) __guaranteed_movsb((unsigned char *)dst, (unsigned char const *)src, len)

/*
 * WARNING: Copies exactly 16 bytes, regardless of the actual valid data range.
 *  This WILL certainly lead to crashes in situations where the memory becomes invalid,
 *  after a certain size has been surpassed.
*/
LLVM_NOOPT inline void memcpy_16(unsigned char* dst, const unsigned char* src)
{
    __m128i xmm0 = _mm_loadu_si128((__m128i*)src);
    _mm_storeu_si128((__m128i*)dst, xmm0);
}

/*
 * WARNING: Hashes exactly 16 bytes, regardless of the actual valid data range.
 *  This WILL certainly lead to crashes in situations where the memory becomes invalid,
 *  after a certain size has been surpassed.
*/
LLVM_NOOPT inline void str2hash_16(uint64_t* out, const char* str)
{
    __m128i rs;
    memcpy_16(reinterpret_cast<unsigned char*>(&rs), reinterpret_cast<const unsigned char*>(str));

    out[0] = _mm_cvtsi128_si64(rs);
    out[1] = _mm_cvtsi128_si64(_mm_srli_si128(rs, 8));
}

inline constexpr uint32_t float2dword(const float f)
{
    return __builtin_bit_cast(uint32_t, f);
}

__attribute((always_inline)) inline float dword2float(uint32_t dw)
{
    float rs = 0.f;
    memcpy(&rs, &dw, 4);
    return rs;
}

#define _fp(name, val)                          \
    constexpr auto __##name = float2dword(val); \
    auto name = dword2float(__##name);

#define _fpg(name, val)                                       \
    inline static constexpr auto __##name = float2dword(val); \
    inline static auto name = dword2float(__##name);

#include "stl/linked_list_single.h"
