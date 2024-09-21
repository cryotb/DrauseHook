#pragma once

#define resolve_rva(addr, instr_off, instr_len) ((addr + *reinterpret_cast<int32_t *>(addr + instr_off)) + instr_len)

namespace offsets
{
    namespace detail
    {
        inline uptr find_ptr(uptr start, uptr len, const char* pattern, u16 ioff, u16 ilen)
        {
            auto rs = find_pattern(start, len, pattern);
            if(rs)
            {
                return resolve_rva(rs, ioff, ilen);
            }
            return 0;
        }
    }

    inline u64 fix_scn_off(void *image, u32 sec_base, uptr off)
    {
        //msg("fix_scn_off(%p, %x, %p)", image, sec_base, off);
        return ((BASE_OF(image)) + sec_base + off);
    }

    inline u64 find(u64 base, u64 start, u64 len, const char *name, const char *pattern)
    {
        auto initial_addr = find_pattern(start, len, pattern);
        if (!initial_addr)
        {
            safeLog_msg("!! couldn't find an offset (pattern) -> %s", name);
            exit(0);
        }

        auto corrected = initial_addr - base;
        safeLog_msg("** pattern %s (final) -> %lx", name, corrected);

        return corrected;
    }

    inline u64 find_ptr(u64 base, u64 start, u64 len, const char *name, const char *pattern, u64 ioff, u64 ilen)
    {
        auto initial_addr = find_pattern(start, len, pattern);
        if (!initial_addr)
        {
            safeLog_msg("!! couldn't find an offset (pattern/PTR) -> %s", name);
            exit(0);
        }

        auto corrected = resolve_rva(initial_addr, ioff, ilen);
        if (!corrected)
        {
            safeLog_msg("!! couldn't correct an offset (pattern/PTR) -> %s", name);
            exit(0);
        }

        corrected -= base;
        safeLog_msg("** pattern_PTR %s (final) -> %lx", name, corrected);

        return corrected;
    }

    inline u64 find_local_ptr_adj_remote(u64 start, u64 len, u64 remote_base, 
        u64 remote_scn_base, const char *name, const char *pattern, u64 ioff, u64 ilen, bool fix = true)
    {
        auto initial_addr = find_pattern(start, len, pattern);
        if (!initial_addr)
        {
            safeLog_msg("!! couldn't find an offset (pattern/PTR) -> %s", name);
            exit(0);
        }

        auto corrected = resolve_rva(initial_addr, ioff, ilen);
        if (!corrected)
        {
            safeLog_msg("!! couldn't correct an offset (pattern/PTR) -> %s", name);
            exit(0);
        }

        auto rs = fix ? (fix_scn_off(PTR_OF(remote_base), remote_scn_base, (corrected - start)) - remote_base) : corrected;
        if(!fix) rs -= remote_base;
        safeLog_msg("** pattern_PTR %s (final) -> %lx", name, rs);

        return rs;
    }

    inline u64 find_local_adj_remote(u64 start, u64 len, u64 remote_base, u64 remote_scn_base, const char *name, const char *pattern, bool fix = true)
    {
        auto initial_addr = find_pattern(start, len, pattern);
        if (!initial_addr)
        {
            safeLog_msg("!! couldn't find an offset (pattern) -> %s", name);
            exit(0);
        }

        auto rs = fix ? (fix_scn_off(PTR_OF(remote_base), remote_scn_base, (initial_addr - start)) - remote_base) : initial_addr;
        if(!fix) rs -= remote_base;
        safeLog_msg("** pattern %s (final) -> %lx", name, rs);

        return rs;
    }
}

#define FIND_SIG(BASE, START, LEN, NAME, PAT) offsets::find(BASE, START, LEN, _XS(NAME), _XS(PAT))
#define FIND_PTR(BASE, START, LEN, NAME, PAT, IOFF, ILEN) offsets::find_ptr(BASE, START, LEN, _XS(NAME), _XS(PAT), IOFF, ILEN)

// GIVES AN RVA ONLY, WITH IMAGE BASE SUBTRACTED.
#define FIND_OFF_LAJR(START, LEN, REMOTE_BASE, REMOTE_SCN_BASE, NAME, PAT, FIX) offsets::find_local_adj_remote(START, LEN, REMOTE_BASE, REMOTE_SCN_BASE, _XS(NAME), _XS(PAT), FIX)
#define FIND_PTR_LAJR(START, LEN, REMOTE_BASE, REMOTE_SCN_BASE, NAME, PAT, IOFF, ILEN, FIX) offsets::find_local_ptr_adj_remote(START, LEN, \
REMOTE_BASE, REMOTE_SCN_BASE, _XS(NAME), _XS(PAT), IOFF, ILEN, FIX)
