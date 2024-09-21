#pragma once

namespace rs::netvars
{
    extern bool init();
    extern int32_t find(const char* table, const char* name);
    extern void dump();
}

LLVM_NOOPT LLVM_INLINE void __netvar_assign(const char *table, const char *prop, u32 *receiver)
{
	auto off = rs::netvars::find(table, prop);
	if (off == 0)
	{
	#if LOGGING_IS_ENABLED == 1
		msg("failed to find a netvar!");
	#endif

		__fastfail(0xDAC);
	}
	*receiver = off;
}

#define NETVAR_OFFSET_RECEIVER(NAME) inline u32 NAME
#define NETVAR_ASSIGN(RECEIVER, TABLE_NAME, PROP_NAME) __netvar_assign(TABLE_NAME, PROP_NAME, RECEIVER)
