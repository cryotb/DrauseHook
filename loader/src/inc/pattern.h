#pragma once

#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

namespace sig
{
	OBFUS_FUNC_DISABLE inline
		bool isMatch(const u8* addr, const u8* pat, const u8* msk)
	{
		size_t n = 0;
		while (addr[n] == pat[n] || msk[n] == (u8)'?') {
			if (!msk[++n]) {
				return true;
			}
		}
		return false;
	}

	OBFUS_FUNC_DISABLE inline const u8* find_pattern(const u8* rangeStart, u64 len, const char* pattern)
	{
		size_t l = strlen(pattern);
		u8* patt_base = static_cast<u8*>(alloca(l >> 1));
		u8* msk_base = static_cast<u8*>(alloca(l >> 1));
		u8* pat = patt_base;
		u8* msk = msk_base;
		l = 0;
		while (*pattern) {
			if (*pattern == ' ')
				pattern++;
			if (!*pattern)
				break;
			if (*(u8*)pattern == (u8)'\?') {
				*pat++ = 0;
				*msk++ = '?';
				pattern += ((*(u16*)pattern == (u16)'\?\?') ? 2 : 1);
			}
			else {
				*pat++ = getByte(pattern);
				*msk++ = 'x';
				pattern += 2;
			}
			l++;
		}
		*msk = 0;
		pat = patt_base;
		msk = msk_base;
		for (u32 n = 0; n < (len - l); ++n)
		{
			if (isMatch(rangeStart + n, patt_base, msk_base)) {
				return rangeStart + n;
			}
		}
		return NULL;
	}

	OBFUS_FUNC_DISABLE inline auto get_pattern_byte_length(const char* pattern)
	{
		size_t l = strlen(pattern);
		u8* patt_base = static_cast<u8*>(alloca(l >> 1));
		u8* msk_base = static_cast<u8*>(alloca(l >> 1));
		u8* pat = patt_base;
		u8* msk = msk_base;
		u64 result = 0;

		int fields = 0;
		for (u64 i = 0; i < l; i++)
		{
			if (pattern[i] == ' ')
				continue;

			if (fields < 1)
				fields++;
			else
			{
				result++;
				fields = 0;
			}
		}

		return result;
	}

	OBFUS_FUNC_DISABLE inline auto find_pattern_rc(const u8* rangeStart, u64 len, const char* pattern)
	{
		auto result = std::vector<u64>();
		auto pat_len = get_pattern_byte_length(pattern);

		if (pat_len > 0)
		{
			const u8* last_hit = nullptr;

			for (auto curr = rangeStart; (curr < (rangeStart + len)); )
			{
				const u8* rs = nullptr;

				if (!last_hit)
					rs = find_pattern(curr, len, pattern);
				else
				{
					rs = find_pattern(curr, ((rangeStart + len) - last_hit - pat_len), pattern);
				}

				if (rs)
				{
					result.push_back(BASE_OF(rs));
					curr = rs + pat_len;
					last_hit = rs;
				}
				else
					break;
			}
		}

		return result;
	}
}

#define find_pattern(base, length, pat) (u64)sig::find_pattern((u8*)base, length, pat)
