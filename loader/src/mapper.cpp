#include "inc/include.h"

uptr get_directory_file_addr(uptr base, PIMAGE_NT_HEADERS64 nt_hdr, u8 index)
{
    auto dir = &nt_hdr->OptionalHeader.DataDirectory[index];
    if (!dir->VirtualAddress || dir->Size < 1)
    {
        return 0;
    }

    auto dir_ptr = pe::get_ptr_from_va(dir->VirtualAddress, nt_hdr, (u8 *)base);
    if (dir_ptr == nullptr)
    {
        return 0;
    }

    return (uptr)dir_ptr;
}

bool erase_directory(uptr base, PIMAGE_NT_HEADERS64 nt_hdr, u8 index)
{
    auto dir = &nt_hdr->OptionalHeader.DataDirectory[index];
    if (!dir->VirtualAddress || dir->Size < 1)
    {
        safeLog_msg("[erase-directory]: given index is not present.");
        return false;
    }

    auto dir_ptr = pe::get_ptr_from_va(dir->VirtualAddress, nt_hdr, (u8 *)base);
    if (dir_ptr == nullptr)
    {
        safeLog_msg("[erase-directory]: could not resolve pointer.");
        return false;
    }

    memset(dir_ptr, 0, dir->Size);

    safeLog_msg("[erase-directory]: handled directory %i with base %p and length %llx",
        index, dir_ptr, dir->Size);

    return true;
}

bool wipe_sensitive_data(uintptr_t base, PIMAGE_DOS_HEADER dos_hdr, PIMAGE_NT_HEADERS64 nt_hdr)
{
    if (!erase_directory(base, nt_hdr, IMAGE_DIRECTORY_ENTRY_EXPORT))
    {
        safeLog_msg("failed to erase sensitive directories.");
        return false;
    }

    auto debug_dir_ptr = (PIMAGE_DEBUG_DIRECTORY)get_directory_file_addr(base, nt_hdr, IMAGE_DIRECTORY_ENTRY_DEBUG);
    if (debug_dir_ptr)
    {
        safeLog_msg("ddptr:%p", debug_dir_ptr);
        auto debug_data = pe::get_ptr_from_va(debug_dir_ptr->AddressOfRawData, nt_hdr, (u8 *)base);

        safeLog_msg("dataptr:%p", debug_data);
        safeLog_msg("len: %x", debug_dir_ptr->SizeOfData);

        memset(debug_data, 0, debug_dir_ptr->SizeOfData);

        if (!erase_directory(base, nt_hdr, IMAGE_DIRECTORY_ENTRY_DEBUG))
        {
            safeLog_msg("failed to erase sensitive directories.");
            return false;
        }
    }
    else
    {
        safeLog_msg("debug dir does not exist.");
    }

    auto image_len = nt_hdr->OptionalHeader.SizeOfImage;
    auto sizeof_hdrs = nt_hdr->OptionalHeader.SizeOfHeaders;
    safeLog_msg("size of headers is %llx", sizeof_hdrs);

    if (sizeof_hdrs > 0)
        RAND_bytes((unsigned char *)base, sizeof_hdrs);

#if !defined(R5I_PROD)
    if (!tools::aob_write_to_disk(_XS("/home/drof/Documents/wiped.bin"), (void *)base, image_len))
    {
        safeLog_msg("failed to write wiped image onto disk.");
        return false;
    }
#endif

    return true;
}

relocation_list mapper::get_relocation_list(u64 image_base, PIMAGE_NT_HEADERS64 nt_header)
{
    relocation_list result;

    if (nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0)
        return result;

    PIMAGE_BASE_RELOCATION relocationTable =
        (PIMAGE_BASE_RELOCATION)(image_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

    DWORD relocationEntriesCount = 0;
    PBASE_RELOCATION_ENTRY relocationRVA = NULL;

    while (relocationTable->SizeOfBlock > 0)
    {
        relocationEntriesCount = (relocationTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
        relocationRVA = (PBASE_RELOCATION_ENTRY)(relocationTable + 1);

        for (uint32_t i = 0; i < relocationEntriesCount; i++)
        {
            if (relocationRVA[i].Offset)
            {
                auto &entry = result.emplace_back();

                entry.m_type = relocationRVA[i].Type;
                entry.m_address = image_base + relocationTable->VirtualAddress + relocationRVA[i].Offset;
            }
        }

        relocationTable = (PIMAGE_BASE_RELOCATION)((u64)relocationTable + relocationTable->SizeOfBlock);
    }

    return result;
}

bool index_game_build(int pid, u64 game_image_base, u64 game_image_len, u64 cs_base, u64 cs_len)
{
    auto remote_shdr_text = gctx->m_game_secs.get_sec_hdr(".text");
    if (!remote_shdr_text.VirtualAddress)
    {
        safeLog_msg("failed to grab remote segment header for TEXT.");
        return false;
    }

    auto game_cs_base_remote = remote_shdr_text.VirtualAddress;

    auto version_base = FIND_PTR_LAJR(cs_base, cs_len, game_image_base, game_cs_base_remote,
                                      "game_version", "48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 1A", 3, 7, true);
    if (!version_base)
        return false;

    safeLog_msg("version_base_off= %lx", version_base);
    safeLog_msg("waiting for game version data to be populated...");

    char version[16];
    ZERO_MEM(version, sizeof(version));

    do
    {
        if (!gmm->read(version, (gctx->m_game_base + version_base), sizeof(version)))
        {
            safeLog_msg("index_game_build: failed to read version base.");
            return false;
        }

        if (strstr(version, "v") && strstr(version, "."))
        {
            safeLog_msg("found a valid game version info!");
            break;
        }

        usleep(500000);
    } while (true);

    auto build_id_base = FIND_PTR_LAJR(cs_base, cs_len, game_image_base, game_cs_base_remote,
                                       "build_id", "48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 88 05 ? ? ? ? 84 C0 75 44 48 8B 0D", 3, 7, true);
    if (!build_id_base)
        return false;

    safeLog_msg("build_id_base_off= %lx", build_id_base);

    char build_id[64];
    ZERO_MEM(build_id, sizeof(build_id));

    if (!gmm->read(build_id, (gctx->m_game_base + build_id_base), sizeof(build_id)))
    {
        safeLog_msg("index_game_build: failed to read build_id base.");
        return false;
    }

    safeLog_msg("------ GAME BUILD INFO BELOW ------");
    safeLog_msg("version  = '%s'", version);
    safeLog_msg("build_id = '%s'", build_id);
    safeLog_msg("-----------------------------------");

    if (gctx->m_opts.dump_game_image)
    {
        char dump_path[256];
        ZERO_MEM(dump_path, sizeof(dump_path));

        sprintf(dump_path, _XS("/home/drof/Documents/(%s)_%s"), version, build_id);

        if (!tools::aob_write_to_disk(dump_path, PTR_OF(game_image_base), game_image_len))
        {
            safeLog_msg("index_game_build: failed to dump game image.");
            return false;
        }

        safeLog_msg("index_game_build: dump successful!");
    }

    return true;
}

bool create_memdump_from_remote_image(uptr base, std::vector<u8>& buf)
{
    std::vector<u8> headers(0x1000);

    if(!gmm->read(headers.data(), base, headers.size()))
        return false;

    auto image = headers.data();
    auto dh = reinterpret_cast<PIMAGE_DOS_HEADER>(image);
    auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(BASE_OF(image) + dh->e_lfanew);

    if(dh->e_magic != IMAGE_DOS_SIGNATURE || nh->Signature != IMAGE_NT_SIGNATURE)
        return false;

    auto length = nh->OptionalHeader.SizeOfImage;
    if(length < 1)
        return false;

    buf.resize( length );
    return gmm->read(buf.data(), base, buf.size());
}

bool resolve_winapis()
{
    const auto handle_kernel32 = []()
    {
        std::vector<u8> kernel32_dump;
        if (!create_memdump_from_remote_image(gctx->m_kernel32_base, kernel32_dump))
        {
            safeLog_msg("failed to read kernel32 headers.");
            return false;
        }

        auto image = kernel32_dump.data();
        auto dh = (PIMAGE_DOS_HEADER)image;
        if (dh->e_magic != IMAGE_DOS_SIGNATURE)
        {
            safeLog_msg("invalid kernel32 DOS header.");
            return false;
        }

        auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(BASE_OF(image) + dh->e_lfanew);
        if (nh->Signature == IMAGE_NT_SIGNATURE)
        {
            gctx->m_lctx.winapis.create_file = pe::find_export_address_virt(BASE_OF(image), dh, nh, "CreateFileA") - BASE_OF(image) + gctx->m_kernel32_base;
            gctx->m_lctx.winapis.get_file_size = pe::find_export_address_virt(BASE_OF(image), dh, nh, "GetFileSize") - BASE_OF(image) + gctx->m_kernel32_base;
            gctx->m_lctx.winapis.close_handle = pe::find_export_address_virt(BASE_OF(image), dh, nh, "CloseHandle") - BASE_OF(image) + gctx->m_kernel32_base;
            gctx->m_lctx.winapis.read_file = pe::find_export_address_virt(BASE_OF(image), dh, nh, "ReadFile") - BASE_OF(image) + gctx->m_kernel32_base;

            safeLog_msg("[winapis] CreateFile: %p", gctx->m_lctx.winapis.create_file);
            safeLog_msg("[winapis] GetFileSize: %p", gctx->m_lctx.winapis.get_file_size);
            safeLog_msg("[winapis] CloseHandle: %p", gctx->m_lctx.winapis.close_handle);
            safeLog_msg("[winapis] ReadFile: %p", gctx->m_lctx.winapis.read_file);
        }

        return true;
    };

    const auto handle_ntdll = []()
    {
        std::vector<u8> dump;
        if (!create_memdump_from_remote_image(gctx->m_ntdll_base, dump))
        {
            safeLog_msg("failed to read NTDLL headers.");
            return false;
        }

        auto image = dump.data();
        auto dh = (PIMAGE_DOS_HEADER)image;
        if (dh->e_magic != IMAGE_DOS_SIGNATURE)
        {
            safeLog_msg("invalid NTDLL DOS header.");
            return false;
        }

        auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(BASE_OF(image) + dh->e_lfanew);
        if (nh->Signature == IMAGE_NT_SIGNATURE)
        {
            gctx->m_lctx.winapis.capture_stack_back_trace = pe::find_export_address_virt(BASE_OF(image), dh, nh, "RtlCaptureStackBackTrace") - BASE_OF(image) + gctx->m_ntdll_base;

            safeLog_msg("[winapis] CaptureStackBackTrace: %p", gctx->m_lctx.winapis.capture_stack_back_trace);
        }

        return true;
    };


    if (!handle_kernel32())
        return false;

    if(!handle_ntdll())
        return false;

    return true;
}

template <typename T>
bool __verbose_assign(const char *caller, const char *name, T &dst, uptr value)
{
    if (value == 0)
    {
        safeLog_msg("%s -> failed to find %s!", caller, name);
        return false;
    }

    dst = value;

    safeLog_msg("%s -> resolved %s to %p", caller, name, value);
    return true;
};

#define XVERBOSE_ASSIGN(NAME, TYPE, DST, VAL) __verbose_assign<TYPE>(__FUNCTION__, NAME, DST, VAL)
#define XVERBOSE_RESOLVE(NAME, TYPE, DST) XVERBOSE_ASSIGN(NAME, TYPE, DST, image.find_symbol(NAME))

u8 *lcrt_resolve_math_wrapper1(void* image_ptr, LoaderContext& lctx, u8* head)
{
    auto curr = head;
    bool found = false;

    while ((curr - head) < 32)
    {
        if (curr[0] == 0x48 && curr[1] == 0x8D)
        {
            found = true;
            break;
        }

        ++curr;
    }

    if (!found)
    {
        safeLog_msg("failed to analyze sinf32 (1).");
        return nullptr;
    }

    auto resolved_addr = mem::resolve_operandof_lea(curr);
    if (!resolved_addr)
    {
        safeLog_msg("failed to analyze sinf32 (2).");
        return nullptr;
    }

    return reinterpret_cast<u8*>(resolved_addr);
}

bool mapper::resolve_linux_crt()
{
    safeLog_msg("resolving the linux CRT...");

    auto &lctx = gctx->m_lctx;

    const auto handle_libcpp = [&]()
    {
        auto image = ELFImage("/lib/libc.so.6");
        auto image_ptr = image.buf();

        auto scn_text = image.find_section(".text");
        auto scn_text_begin = BASE_OF(image_ptr) + scn_text->addr;

        safeLog_msg("libc TEXT (%llx, L%llx)", scn_text->addr, scn_text->length);

        auto libc_syscall_gadget = FIND_SIG(BASE_OF(image_ptr),
                                            scn_text_begin, scn_text->length, "libc_syscall_gadget_offset", "0F 05");
        gctx->m_libc_syscall_gadget_offset = libc_syscall_gadget;
        if (!libc_syscall_gadget)
            return false;

        if (!XVERBOSE_RESOLVE("fopen", u32, lctx.linuxcrt_cpp.fopen))
            return false;
        if (!XVERBOSE_RESOLVE("fputs", u32, lctx.linuxcrt_cpp.fputs))
            return false;
        if (!XVERBOSE_RESOLVE("fclose", u32, lctx.linuxcrt_cpp.fclose))
            return false;
        if (!XVERBOSE_RESOLVE("sprintf", u32, lctx.linuxcrt_cpp.sprintf))
            return false;
        if (!XVERBOSE_RESOLVE("sleep", u32, lctx.linuxcrt_cpp.sleep))
            return false;

        return true;
    };

    const auto handle_libmath = [&]()
    {
        auto image = ELFImage("/lib/libm.so.6");
        auto image_ptr = image.buf();

        auto scn_text = image.find_section(".text");
        auto scn_text_begin = BASE_OF(image_ptr) + scn_text->addr;
        safeLog_msg("libm TEXT (%llx, L%llx)", scn_text->addr, scn_text->length);

        if (!XVERBOSE_RESOLVE("acosf", u32, lctx.linuxcrt_math.acosf))
            return false;
        if (!XVERBOSE_RESOLVE("atan2f", u32, lctx.linuxcrt_math.atan2f))
            return false;
        if (!XVERBOSE_RESOLVE("fabsf", u32, lctx.linuxcrt_math.fabsf))
            return false;

        if (XVERBOSE_RESOLVE("sinf32", u32, lctx.linuxcrt_math.sinf))
        {
            auto head = reinterpret_cast<u8 *>(BASE_OF(image_ptr) + lctx.linuxcrt_math.sinf);
            auto resolved_addr = lcrt_resolve_math_wrapper1(image_ptr, lctx, head);
            auto resolved_rva = resolved_addr - BASE_OF(image_ptr);
            if(!resolved_addr)
            {
                safeLog_msg("failed to analyze sinf32 (2).");
                return false;
            }

            safeLog_msg("DYN_sinf: %p", resolved_rva);
        } else
            return false;

        if (XVERBOSE_RESOLVE("cosf32", u32, lctx.linuxcrt_math.cosf))
        {
            auto head = reinterpret_cast<u8 *>(BASE_OF(image_ptr) + lctx.linuxcrt_math.cosf);
            auto resolved_addr = lcrt_resolve_math_wrapper1(image_ptr, lctx, head);
            auto resolved_rva = resolved_addr - BASE_OF(image_ptr);
            if (!resolved_addr)
            {
                safeLog_msg("failed to analyze cosf (2).");
                return false;
            }

            safeLog_msg("DYN_cosf: %p", resolved_rva);
        }
        else
            return false;

        return true;
    };

    if (!handle_libcpp())
    {
        safeLog_msg("mapper::resolve_linux_crt() -> failed to handle LIBCPP.");
        return false;
    }

    safeLog_msg("-------------------------");

    if (!handle_libmath())
    {
        safeLog_msg("mapper::resolve_linux_crt() -> failed to handle LIBMATH.");
        return false;
    }

    return true;
}

#include <sys/stat.h>

int aob_map_file(const char* file_path, void** mapped_data, off_t* file_size) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        safeLog_msg("Error opening file");
        return 1;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        close(fd);
        safeLog_msg("Error getting file size");
        return 1;
    }

    *file_size = file_stat.st_size;

    *mapped_data = mmap(NULL, *file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (*mapped_data == MAP_FAILED) {
        close(fd);
        safeLog_msg("Error mapping file into memory");
        return 1;
    }

    close(fd);
    return 0;
}

bool mapper::setup_pre()
{
    if (gctx->m_opts.analysis_from_dump)
    {
        /* !! MUST BE A VIRTUAL DUMP !! */
        void* gsnap = nullptr;
        off_t gsnap_len = 0;

        if (!gctx->m_opts.analysis_from_dump_vec)
        {
            if (aob_map_file(gctx->m_opts.analysis_from_dump_path.c_str(), &gsnap, &gsnap_len) != 0)
            {
                safeLog_msg("failed to read game snapshot.");
                return false;
            }
        }
        else
        {
            gsnap = gctx->m_opts.vec_analysis_from_dump.data();
            gsnap_len = gctx->m_opts.vec_analysis_from_dump.size();
        }

        auto gsnap_base = BASE_OF(gsnap);
        auto dh = reinterpret_cast<PIMAGE_DOS_HEADER>(gsnap);
        auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(gsnap_base + dh->e_lfanew);

        if (dh->e_magic != IMAGE_DOS_SIGNATURE)
        {
            safeLog_msg("failed to very SIG_DOS for game snapshot.");
            return false;
        }

        if (nh->Signature != IMAGE_NT_SIGNATURE)
        {
            safeLog_msg("failed to verify SIG_NT for game snapshot");
            return false;
        }

        auto sec_cs = pe::find_section_base_virt(gsnap, ".text");
        auto cs_base = gsnap_base + sec_cs->VirtualAddress;
        auto cs_len = sec_cs->Misc.VirtualSize;

        auto lctx = &gctx->m_lctx;

        if (!respawn::collect_offsets(gctx, lctx, gsnap_base, cs_base, cs_len, true))
        {
            safeLog_msg("failed to collect respawn offsets!");
            return false;
        }
    }

     gctx->m_strtbl = string_table::Create({
            "ANON",
            "VIS",
            " FAR",
            " OUL",
            "L",
            "R",
            "RCO",
            " MDS",
            " STR",
            "AA",
            " SLO",
            " SLOSTR",
            " ROR",
            "  FOV",
            "  DZ",
            "RAO",
            "Build ID: %s",
            "%s | %s",
            "%s (%.1f)",
            "%s (%.2f)",
            "%s (%.3f)",
            "%s (%.4f)",
            "%s (%.0f)",
            "TGB",
        });

        auto strtbl_length = gctx->m_strtbl->table_total_length;

#if !defined(R5I_PROD)
        string_table::Dump_records(gctx->m_strtbl);
#endif

        string_table::Encrypt(gctx->m_strtbl);

#if !defined(R5I_PROD)
        string_table::Dump_contents(gctx->m_strtbl, strtbl_length);
#endif

    return true;
}

bool mapper::setup(int pid)
{
    gctx->m_pid = pid;
    gctx->m_game_base = tools::get_mapping_base(pid, MAPPING_NAME);
    if (!gctx->m_game_base)
        return false;

    safeLog_msg("base is %p", gctx->m_game_base);

    gctx->m_libc_base = tools::get_mapping_base(pid, _XS("libc.so.6"));
    gctx->m_kernel32_base = tools::get_mapping_base(pid, _XS("kernel32.dll"));
    gctx->m_ntdll_base = tools::get_mapping_base(pid, _XS("ntdll.dll"));
    safeLog_msg("base of libc is %p", gctx->m_libc_base);
    safeLog_msg("base of kernel32 is %p", gctx->m_kernel32_base);
    safeLog_msg("base of ntdll is %p", gctx->m_ntdll_base);
    
    if (!resolve_winapis())
    {
        safeLog_msg("failed to resolve winAPIs.");
        return false;
    }

    if (!pe_dump_image(pid, gctx->m_game_base, &gctx->m_game_len))
    {
        safeLog_msg("failed to dump the PE image.");
        return false;
    }

    auto game_sec_cs = gctx->m_game_secs.snap_cs;
    auto game_cs_base = BASE_OF(game_sec_cs.data());
    auto game_cs_len = game_sec_cs.size();

    safeLog_msg("game_cs_base -> %lx L%lx", game_cs_base, game_cs_len);

    auto lctx = &gctx->m_lctx;

    lctx->magic = 'BY';
    lctx->game_base = gctx->m_game_base;
    lctx->game_len = gctx->m_game_len;

    auto game_sec_hdr_rdata = gctx->m_game_secs.get_sec_hdr(".rdata");
    if(!game_sec_hdr_rdata.VirtualAddress || game_sec_hdr_rdata.Misc.VirtualSize < 1)
    {
        safeLog_msg("failed to find game RDATA section HDR.");
        return false;
    }
    lctx->game_rdata_base = gctx->m_game_base + game_sec_hdr_rdata.VirtualAddress;
    lctx->game_rdata_len = game_sec_hdr_rdata.Misc.VirtualSize;

    auto game_sec_hdr_cs = gctx->m_game_secs.get_sec_hdr(".text");
    if(!game_sec_hdr_cs.VirtualAddress || game_sec_hdr_cs.Misc.VirtualSize < 1)
    {
        safeLog_msg("failed to find game TEXT section HDR.");
        return false;
    }
    lctx->game_cs_base = gctx->m_game_base + game_sec_hdr_cs.VirtualAddress;
    lctx->game_cs_len = game_sec_hdr_cs.Misc.VirtualSize;

    auto game_sec_hdr_data = gctx->m_game_secs.get_sec_hdr(".data");
    if(!game_sec_hdr_data.VirtualAddress || game_sec_hdr_data.Misc.VirtualSize < 1)
    {
        safeLog_msg("failed to find game DATA section HDR.");
        return false;
    }
    lctx->game_data_base = gctx->m_game_base + game_sec_hdr_data.VirtualAddress;
    lctx->game_data_len = game_sec_hdr_data.Misc.VirtualSize;

    lctx->libc_base = gctx->m_libc_base;
    lctx->libm_base = tools::get_mapping_base(pid, _XS("libm.so.6"));

    if (!gctx->m_opts.analysis_from_dump)
    {
        if (!respawn::collect_offsets(gctx, lctx, gctx->m_game_base, game_cs_base, game_cs_len))
        {
            safeLog_msg("failed to collect respawn offsets!");
            return false;
        }
    }

    if (gctx->m_opts.debug_sigs)
    {
        safeLog_msg("!! sig debug mode is enabled, quitting now.");
        exit(0);
    }

    return true;
}

bool mapper::fetch_image(std::vector<u8>& buf)
{
    auto image = buf.data();

    auto dh = (PIMAGE_DOS_HEADER)image;
    auto nh = reinterpret_cast<PIMAGE_NT_HEADERS64>(BASE_OF(image) + dh->e_lfanew);

    if (dh->e_magic != IMAGE_DOS_SIGNATURE || nh->Signature != IMAGE_NT_SIGNATURE)
    {
        safeLog_msg("fetch-image: invalid headers!");
        return false;
    }

    gctx->m_payload.entry = nh->OptionalHeader.AddressOfEntryPoint;
    gctx->m_payload.length = nh->OptionalHeader.SizeOfImage;
    safeLog_msg("payload entry -> 0x%x", gctx->m_payload.entry);
    safeLog_msg("payload len -> 0x%x", gctx->m_payload.length);

    auto sh = IMAGE_FIRST_SECTION64(nh);

    for (u16 i = 0; i < nh->FileHeader.NumberOfSections; i++)
    {
        auto sec = &sh[i];
        auto name = (char *)sec->Name;

        bool add = false;

        if (!sec->PointerToRawData && !sec->SizeOfRawData)
        {
            safeLog_msg("skipping an empty section %s", name);
            continue;
        }

        if (strcmp(name, ".text") == 0 || strcmp(name, ".data") == 0 || strcmp(name, ".rdata") == 0)
            add = true;

        if (!add)
            continue;

        safeLog_msg(" %i added to copy list -> %s (%llx, %x)", i, name, sec->PointerToRawData, sec->Misc.VirtualSize);
        gctx->m_payload.sections.push_back(*sec);
    }

    gctx->m_hooks.fn_init = pe::find_export_address(BASE_OF(image), dh, nh, "_kb", false) - BASE_OF(image);
    safeLog_msg("** _kb -> %llx", gctx->m_hooks.fn_init);

    gctx->m_payload.ptr_new_loader_ctx = mm::aob_search_buf_single_u64(image, buf.size(), 0xAAAAAAAAAAAAAAAC);
    if (!gctx->m_payload.ptr_new_loader_ctx)
        return false;
    safeLog_msg("fetch-image: found new_loader_ctx at %llx", (gctx->m_payload.ptr_new_loader_ctx - BASE_OF(image)));

    //
    // After being done with exports, wipe them.
    //
    if (!wipe_sensitive_data(BASE_OF(image), dh, nh))
    {
        safeLog_msg("failed to wipe sensitive payload data.");
        return false;
    }

    return true;
}

bool restore_instance_vft(uptr inst, uptr vft_orig)
{
    if (!gmm->twrite<u64>(inst, vft_orig))
    {
        safeLog_msg("restore_instance_vft: failed to update vft for %p.", inst);
        return false;
    }

    if (gmm->tread<u64>(inst) != vft_orig)
    {
        safeLog_msg("restore_instance_vft: the new vft base doesn't match with what i wrote for %p.", inst);
        return false;
    }

    safeLog_msg("restored vft for %p with success, its vft now points to %p", inst, gmm->tread<u64>(inst));

    return true;
}

#define VFT_INDEX_INIT 4
bool prepare_vfts()
{
    u64 base_client = gctx->m_game_base + gctx->m_offsets.iface_base_client;
    gctx->m_vft_orig_ptrs.base_client = gmm->tread<u64>(gctx->m_ix.base_client);

    gctx->m_lctx.offsets.base_client = gctx->m_offsets.iface_base_client;
    gctx->m_lctx.vft_orig_ptrs.base_client = gctx->m_vft_orig_ptrs.base_client;

    std::vector<u64> base_client_vft_dump, base_client_vft_dump_orig;

    if (!make_remote_vft(base_client, gctx->m_vfts_remote.base_client, base_client_vft_dump, base_client_vft_dump_orig,
                         {
                             {VFT_INDEX_INIT, gctx->m_shell_base + gctx->m_hooks.fn_init},
                         }))
        return false;

    gctx->m_lctx.origs.init = base_client_vft_dump_orig[VFT_INDEX_INIT];

    safeLog_msg("base_client: init_func orig is at %p", gctx->m_lctx.origs.init);
    safeLog_msg("base_client: setting the init_func handler to %lx", gctx->m_shell_base + gctx->m_hooks.fn_init);
    safeLog_msg("wrote vft OK for base_client.");

    return true;
}

bool place_vfts()
{
    if (!gmm->twrite<u64>(gctx->m_ix.base_client, gctx->m_vfts_remote.base_client))
    {
        safeLog_msg("failed to update vft for base_client!");
        return false;
    }

    return true;
}

bool restore_vfts()
{
    if (!restore_instance_vft(gctx->m_ix.base_client, gctx->m_vft_orig_ptrs.base_client))
    {
        safeLog_msg("failed to restore vft for base_client!");
        return false;
    }

    return true;
}

bool copy_image_sections_to_remote(void *local_image, u64 shell_base)
{
    for (const auto &sec : gctx->m_payload.sections)
    {
        u64 copy_addr = BASE_OF(local_image) + sec.PointerToRawData;
        u64 dest_addr = shell_base + sec.VirtualAddress;
        u64 copy_len = sec.SizeOfRawData;

        safeLog_msg(" - copying %s from %p into %p.", sec.Name, copy_addr, dest_addr);

        if (!gmm->write(dest_addr, PTR_OF(copy_addr), copy_len))
        {
            safeLog_msg("    copy ERR.");
            return false;
        }

        if (strcmp((char *)sec.Name, ".text") == 0)
        {
            safeLog_msg("    protecting code.");

            if (!pt::mprotect(dest_addr, sec.Misc.VirtualSize, PROT_READ | PROT_EXEC))
            {
                safeLog_msg("failed protect (code).");
                return false;
            }
        }
    }

    return true;
}

bool perform_mapping(uptr map_addr, u8* local_image)
{
    u64 shell_base = map_addr;
    u64 ctx_base = pt::mmap(0, sizeof(LoaderContext),
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    u64 strtbl_base = pt::mmap(0, sizeof(LoaderContext),
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (!shell_base)
    {
        safeLog_msg("failed alloc (shell).");
        return false;
    }

    if (!ctx_base)
    {
        safeLog_msg("failed alloc (ctx).");
        return false;
    }

    if (!strtbl_base)
    {
        safeLog_msg("failed alloc (strtbl).");
        return false;
    }

    if (!gmm->fill(shell_base, 0, gctx->m_payload.length) || !gmm->fill(ctx_base, 0, sizeof(LoaderContext)) ||
        !gmm->fill(strtbl_base, 0, gctx->m_strtbl->table_total_length))
    {
        safeLog_msg("failed to fill memory.");
        return false;
    }

    gctx->m_shell_base = shell_base;
    gctx->m_ctx_base = ctx_base;
    gctx->m_strtbl_base = strtbl_base;

    safeLog_msg("remote shell at %p", shell_base);
    safeLog_msg("remote ctx at %p", ctx_base);
    safeLog_msg("remote strtbl at %p", strtbl_base);

    *(u64 *)gctx->m_payload.ptr_new_loader_ctx = ctx_base;

    if (!copy_image_sections_to_remote(local_image, shell_base))
    {
        safeLog_msg("failed to copy image sections to remote.");
        return false;
    }

    gctx->m_lctx.strtbl = reinterpret_cast<string_table::String_table*>(gctx->m_strtbl_base);

    if (!prepare_vfts())
    {
        safeLog_msg("failed to prepare VFTs.");
        return false;
    }

    if (!gmm->write(gctx->m_ctx_base, &gctx->m_lctx, sizeof(gctx->m_lctx)))
    {
        safeLog_msg("failed to write remote loader context.");
        return false;
    }

    if (!gmm->write(gctx->m_strtbl_base, gctx->m_strtbl, gctx->m_strtbl->table_total_length))
    {
        safeLog_msg("failed to write remote string table.");
        return false;
    }

    return true;
}

bool mapper::load(std::vector<u8> &buf)
{
    auto local_image = buf.data();

    //
    //  Setup Interfaces
    //

    gctx->m_ix.base_client = (gctx->m_game_base + gctx->m_offsets.iface_base_client);

    pt::_pid = gctx->m_pid;
    pt::_gadget = (gctx->m_libc_base + gctx->m_libc_syscall_gadget_offset);

    if (gctx->m_opts.dump_game_image != true)
    {
        uptr map_addr = 0;
        uptr map_len = 0;

        if (!gctx->m_opts.use_unsafe_memory)
        {
            uptr compstui_base = 0;
            do
            {
                compstui_base = tools::get_mapping_base(gctx->m_pid, _XS("compstui.dll"));
            } while (!compstui_base);
            safeLog_msg("compstui at %p", compstui_base);

            map_len = 0xEC40;
            map_addr = compstui_base + 0x4000;
        } else
        {
            safeLog_msg("!!!!!! USING UNSAFE MEMORY, ARE YOU FUCKING CRAZY ?! !!!!!!!");
            map_len = gctx->m_payload.length;

            if (!pt::attach(pt::_pid))
            {
                safeLog_msg("load-image: PT failed attach.");
                return false;
            }

            map_addr = pt::mmap(0, map_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            safeLog_msg("maplen=%llx", map_len);
            safeLog_msg("mapaddr: %p", map_addr);

            if (!pt::detach(pt::_pid))
            {
                safeLog_msg("load-image: PT failed detach.");
                return false;
            }
        }

        if (!gmm->fill(map_addr, 0x00, map_len))
        {
            safeLog_msg("load-image: failed fill.");
            return false;
        }

        if (map_len < gctx->m_payload.length)
        {
            safeLog_msg("load-image: space is too small for payload.");
            return false;
        }

        do
        {
            safeLog_msg("waiting for valid bc...");
        } while (gmm->tread<u64>(gctx->m_ix.base_client) == 0);

        if (!pt::attach(pt::_pid))
        {
            safeLog_msg("load-image: PT failed attach.");
            return false;
        }

        perform_mapping(map_addr, local_image);

        if (!pt::detach(pt::_pid))
        {
            safeLog_msg("load-image: PT failed detach.");
            return false;
        }

        if (!place_vfts())
        {
            safeLog_msg("failed to place vfts.");
            return false;
        }
        safeLog_msg("vft have been placed.");
    }
    else
    {
        auto game_sec_cs = gctx->m_game_secs.snap_cs;
        auto game_sec_cs_ptr = game_sec_cs.data();
        auto game_sec_cs_len = game_sec_cs.size();

        if (!index_game_build(
                gctx->m_pid,
                BASE_OF(gctx->m_snap_game.data()),
                gctx->m_snap_game.size(),
                BASE_OF(game_sec_cs_ptr),
                game_sec_cs_len))
        {
            safeLog_msg("failed to index game build!");
            return false;
        }
    }

    return true;
}
