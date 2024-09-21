#include "inc/include.h"

/*
 * Relocates some section details so that the dump can be analyzed statically.
 * IDA and other tools will fuck up if you don't do this on a direct memory dump.
 */
void pe_dump_fix(void *image)
{
    PIMAGE_DOS_HEADER dh = (PIMAGE_DOS_HEADER)image;
    PIMAGE_NT_HEADERS64 nh = (PIMAGE_NT_HEADERS64)(BASE_OF(image) + dh->e_lfanew);

    if (dh->e_magic != IMAGE_DOS_SIGNATURE)
    {
        safeLog_msg("[pe-dump-fix] provided image dump contains invalid DOS header.");
        return;
    }

    if (nh->Signature != IMAGE_NT_SIGNATURE)
    {
        safeLog_msg("[pe-dump-fix] provided image dump contains invalid NT header.");
        return;
    }

    PIMAGE_SECTION_HEADER shdr = IMAGE_FIRST_SECTION64(nh);
    u32 scnt = nh->FileHeader.NumberOfSections;

    for (u32 i = 0; i < scnt; i++)
    {
        PIMAGE_SECTION_HEADER sec = &shdr[i];

        // msg("  fixing %i:%s", i, sec->Name);

        sec->PointerToRawData = sec->VirtualAddress;
        sec->SizeOfRawData = sec->Misc.VirtualSize;
    }
}

bool pe_dump_image(int pid, u64 base, u64* out_len)
{
    IMAGE_DOS_HEADER dh;
    ZERO_MEM(&dh, sizeof(dh));

    if (!mm::detail::vm_read(pid, &dh, base, sizeof(dh)))
    {
        safeLog_msg("[pe-dump] failed to read DOS header.");
        return false;
    }

    if (dh.e_magic != IMAGE_DOS_SIGNATURE)
    {
        safeLog_msg("[pe-dump] invalid DOS signature.");
        return false;
    }

    IMAGE_NT_HEADERS_GENERIC nhg;
    ZERO_MEM(&nhg, sizeof(nhg));

    if (!mm::detail::vm_read(pid, &nhg, base + dh.e_lfanew, sizeof(nhg)))
    {
        safeLog_msg("[pe-dump] failed to read NT(G) header.");
        return false;
    }

    if (nhg.Signature != IMAGE_NT_SIGNATURE)
    {
        safeLog_msg("[pe-dump] invalid NT(G) signature.");
        return false;
    }

    if (nhg.FileHeader.Machine != 0x8664 /* AMD64 */)
    {
        safeLog_msg("[pe-dump] unsupported image arch.");
        return false;
    }

    IMAGE_NT_HEADERS64 nh;
    if (!mm::detail::vm_read(pid, &nh, base + dh.e_lfanew, sizeof(nh)))
    {
        safeLog_msg("[pe-dump] failed to read NT header.");
        return false;
    }

    safeLog_msg("[pe-dump] seems valid, nos: %i sizeofimg: %llx",
        nh.FileHeader.NumberOfSections,
        nh.OptionalHeader.SizeOfImage);

    *out_len = nh.OptionalHeader.SizeOfImage;

    if (gctx->m_opts.dump_game_image)
    {
        // dump full game image, for analysis.
        size_t dump_len = nh.OptionalHeader.SizeOfImage;
        void *dump = malloc(dump_len);

        if (dump)
        {
            ZERO_MEM(dump, dump_len);

            if (mm::detail::vm_read(pid, dump, base, dump_len))
            {
                pe_dump_fix(dump);
                safeLog_msg("[pe-dump] captured dump into memory.");

                auto &snap = gctx->m_snap_game;
                snap.resize(dump_len);
                memcpy(snap.data(), dump, snap.size());
            }
            else
                safeLog_msg("[pe-dump] failed to capture dump into memory.");

            free(dump);
        }
    }

    // only dump necessary sections.
    auto sec_hdr_base = (base + dh.e_lfanew + 0x18 + nh.FileHeader.SizeOfOptionalHeader);
    auto sec_cnt = nh.FileHeader.NumberOfSections;
    auto sec_hdrs_len = sec_cnt * sizeof(IMAGE_SECTION_HEADER);

    if (sec_cnt > 0)
    {
        auto sec_hdrs = std::vector<IMAGE_SECTION_HEADER>(sec_cnt);

        if (!mm::detail::vm_read(pid, sec_hdrs.data(), sec_hdr_base, sec_hdrs_len))
        {
            safeLog_msg("[pe-dump] failed to dump section header table.");
            return false;
        }

        gctx->m_game_secs.table = sec_hdrs;

        safeLog_msg("[pe-dump] dumped section header table.");

        for (const auto &sec : sec_hdrs)
        {
            auto sname = reinterpret_cast<const char *>(&sec.Name);

            if (!strcmp(sname, ".text"))
            {
                auto sbase = (uptr)sec.VirtualAddress;
                auto slen = sec.Misc.VirtualSize;

                if (sbase && slen > 0)
                {
                    sbase += base;

                    safeLog_msg("[pe-dump] image base= %p", base);
                    safeLog_msg("[pe-dump] found code section, base=%p len=%llx", sbase, slen);

                    auto sdump = std::vector<u8>(slen);
                    
                    if(!mm::detail::vm_read(pid, sdump.data(), sbase, slen))
                    {
                        safeLog_msg("[pe-dump] failed to read a segment.");
                        return false;
                    }

                    gctx->m_game_secs.snap_cs = sdump;
                }
            }
        }

        return true;
    }

    return false;
}

PIMAGE_SECTION_HEADER pe::find_section_base_virt(void *image, const char *name)
{
    auto dh = (PIMAGE_DOS_HEADER)image;
    auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(BASE_OF(image) + dh->e_lfanew);

    if (dh->e_magic != IMAGE_DOS_SIGNATURE || nh->Signature != IMAGE_NT_SIGNATURE)
    {
        safeLog_msg("fetch-image: invalid headers!");
        return nullptr;
    }

    auto sh = IMAGE_FIRST_SECTION64(nh);

    for (u16 i = 0; i < nh->FileHeader.NumberOfSections; i++)
    {
        auto sec = &sh[i];
        auto name = (char *)sec->Name;

        bool add = false;

        if (strcmp(name, ".text") == 0)
            return sec;
    }

    return nullptr;
}
