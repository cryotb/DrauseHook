#pragma once

#define HOOK_IDX_ON_DROPPED_BY_SERVER 2
#define HOOK_IDX_PROCESS_PLAYLIST_OVERRIDE 13 // #STR: "SVC_Playlists"
#define HOOK_IDX_ON_RECEIVED_SERVER_INFO (HOOK_IDX_PROCESS_PLAYLIST_OVERRIDE-3) // #STR: "start caching non-common assets for level %s"
#define HOOK_IDX_MKRCTX_UNK1 70 // 44 8B 84 24 ? ? ? ? 0F 57 C0 F3 0F 10 25 - xref it
#define HOOK_IDX_OVERRIDE_MOUSE 33

namespace hooks
{
    LLVM_NOOPT bool init(); // fires after the game has done base setup.
    LLVM_NOOPT bool init_early(); // fires before the game has done base setup.
    LLVM_NOOPT bool init_deferred(); // fires when first entering a match only once.

    extern bool inited_deferred;
    extern u32 vft_disp;
    extern u32 vft_cave_cap;
    extern uptr vft_cave_begin;
}

extern "C"
{
    extern uptr ghk_orig_mkrctx_unk1;
    extern uptr ghk_orig_on_receive_server_info;
    extern uptr ghk_orig_on_dropped_by_server;
    extern uptr ghk_orig_process_playlist_override;

    extern void tp_mkrctx_unk1();
    extern void tp_on_receive_server_info();
    extern void tp_on_dropped_by_server();
    extern void tp_init();
    extern void tp_process_playlist_override();
    extern void tp_override_mouse();

    LLVM_NOOPT unsigned short __fastcall hk_capture_stack_back_trace(unsigned long frames_to_skip, unsigned long frames_to_capture, void **trace, void *trace_hash);
    LLVM_NOOPT void __fastcall hk_mkrctx_unk1(void* inst, u64 a2, void* retaddr);
    LLVM_NOOPT void __fastcall hk_on_receive_server_info(void* inst, uptr netmsg);
    LLVM_NOOPT void __fastcall hk_on_dropped_by_server(void *this_ptr, char unk_cond_1, const char *reason);
    LLVM_NOOPT void __fastcall hk_process_playlist_override(void* inst, void* net_msg);
    LLVM_NOOPT void __fastcall hk_override_mouse(void *inst, void* retaddr);
}
