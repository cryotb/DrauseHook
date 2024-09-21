#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <iostream>
#include <immintrin.h>
#include <lzma.h>
#include <chrono>
using namespace std::chrono_literals;

#include <errno.h>
#include <sys/uio.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>
#include <elf.h>
#include <openssl/rand.h>
#include <fcntl.h>  // Include this for O_RDONLY
#include <unistd.h> // Include this for open and close
#include <gelf.h>
#include <libelf.h>
#include <sodium.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/nvme_ioctl.h>
#include <mutex>

#include "defs.h"
#include "sockets-wrapper.h"
#include "lzmapp.h"
#include "log.h"
#include <backend/include/defs.h>
#include <backend/include/crc32.h>
#include <backend/include/protocol.h>
#include <backend/include/databuf.h>
#include "murmur.h"
#include "string-table.h"
#include "netsdk.h"
#include "memory.h"
#include "libpmap.h"
#include "hde.h"
#include "xorstr.h"
#include "rotate.h"
#include "ntos.h"
#include "mm.h"
#include "pattern.h"
#include "offsets.h"
#include "pe.h"
#include "wptrace.h"
#include "vmt.h"
#include "tools.h"
#include "elfimage.h"
#include "mapper.h"
#include "console.h"
#include "pump.h"
#include "integrity.h"
#include "hwid.h"

namespace pe
{
    struct Section
    {
        Section()
        {
            begin = 0;
            length = 0;
            end = 0;
            name = "NA";
        }

        Section(u64 b, u64 l, const std::string &n)
        {
            begin = b;
            length = l;
            end = (begin + length);
            name = n;
        }

        u64 begin;
        u64 length;
        u64 end;
        std::string name;
    };
}

struct Context
{
    int m_pid;
    u64 m_game_base;
    u64 m_game_len;
    u64 m_shell_base;
    u64 m_ctx_base;
    u64 m_strtbl_base;
    u64 m_libc_base;
    u64 m_kernel32_base;
    u64 m_ntdll_base;
    u32 m_libc_syscall_gadget_offset;

    std::vector<u8> m_snap_game;

    struct
    {
        std::vector<IMAGE_SECTION_HEADER> table;

        std::vector<u8> snap_cs;

        inline auto get_sec_hdr(const char* sname)
        {
            for(const auto& shdr : table)
            {
                auto name = (const char*)&shdr.Name;

                if(!strcmp(name, sname))
                {
                    return shdr;
                }
            }

            return IMAGE_SECTION_HEADER{ };
        }
    } m_game_secs;

    LoaderContext m_lctx;

    struct Options
    {
        bool payload_debug;
        bool debug_sigs;
        bool dump_game_image;
        bool analysis_from_dump;
        bool use_custom_payload_path;
        bool use_unsafe_memory;
        std::string analysis_from_dump_path;
        std::string custom_payload_path;
        bool analysis_from_dump_vec;
        std::vector<u8> vec_analysis_from_dump;
    } m_opts;

    struct Offsets
    {
        u32 iface_base_client;
    } m_offsets;

    struct Iface
    {
        u64 base_client;
    } m_ix;

    struct Hooks
    {
        u64 fn_init;
    } m_hooks;

    struct VftsRemote
    {
        u64 base_client;
    } m_vfts_remote;
    
    struct
    {
        uptr base_client;
    } m_vft_orig_ptrs;

    struct Payload
    {
        u64 length;
        u64 entry;
        std::vector<IMAGE_SECTION_HEADER> sections;
        u64 ptr_new_loader_ctx;
    } m_payload;

    string_table::String_table* m_strtbl;
};

extern Context *gctx;

#include "respawn.h"

#include "loader.h"
#include "packet-handlers.h"
