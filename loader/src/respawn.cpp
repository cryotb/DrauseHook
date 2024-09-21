#include "inc/include.h"

namespace respawn
{
    namespace find
    {
        uptr wrap(const char *name, uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap, uptr (*func)(uptr, uptr, uptr, bool))
        {
            auto rs = func(game_image_base, game_cs_base, game_cs_len, from_snap);
            if (rs)
            {
                safeLog_msg("[respawn] found offset with name '%s' at %llx", name, rs);
                return rs;
            }

            safeLog_msg("[respawn] failed to find offset with name '%s'.", name);
            return 0;
        }

        uptr name_list(uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap)
        {
            bool fix = false;
            uptr game_cs_base_remote = 0;

            if (!from_snap)
            {
                auto remote_shdr_text = gctx->m_game_secs.get_sec_hdr(".text");
                if (!remote_shdr_text.VirtualAddress)
                {
                    safeLog_msg("failed to grab remote segment header for TEXT.");
                    return false;
                }

                game_cs_base_remote = remote_shdr_text.VirtualAddress;
                fix = true;
            }

            auto start_addr = offsets::detail::find_ptr(game_cs_base, game_cs_len,
                                                        "E8 ? ? ? ? 48 8B CB 48 8B F8 E8 ? ? ? ? 4C 8D 05", 1, 5);

            if (start_addr)
            {
                uptr addr = start_addr;
                auto disasm = hde::disasm_buffer_ex((void *)start_addr, MAX_INSTRUCTION_LEN * 20);
                if (!disasm.empty())
                {
                    for (auto it = disasm.begin(); it != disasm.end(); ++it)
                    {
                        const auto &ins = *it;

                        if (ins.opcode == 0x8D && ins.opcode2 == 0x00)
                        {
                            auto imm = *(u32 *)(&reinterpret_cast<u8 *>(addr)[3]);
                            auto rs = (imm + addr + ins.len) - (fix ? game_cs_base : game_image_base);
                            return fix ? (offsets::fix_scn_off(PTR_OF(game_image_base), game_cs_base_remote, rs) - game_image_base) : rs;
                        }

                        addr += ins.len;
                    }
                }
            }
            return 0;
        }

        uptr player_view_offset(uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap)
        {
            auto start_addr = FIND_SIG(game_image_base, game_cs_base, game_cs_len, "find::player_view_offset::start_addr", "48 83 EC 38 8B 05 ? ? ? ? 0F 29 74 24 ? 83 F8 FF 74");
            if (start_addr)
            {
                start_addr += game_image_base;
                uptr addr = start_addr;
                auto disasm = hde::disasm_buffer_ex((void *)start_addr, MAX_INSTRUCTION_LEN * 20);
                if (!disasm.empty())
                {
                    for (const auto &ins : disasm)
                    {
                        if (ins.opcode == 0x0F && ins.opcode2 == 0x10)
                        {
                            auto result = *reinterpret_cast<u32 *>(addr + 5);
                            return result;
                        }

                        addr += ins.len;
                    }
                }
            }
            return 0;
        }

        uptr player_view_angle(uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap)
        {
            auto start_addr = FIND_PTR(game_image_base, game_cs_base, game_cs_len, "find::player_view_angle::start_addr", "E8 ? ? ? ? 80 BB ? ? ? ? ? 74 71 8B 83", 1, 5);
            if (start_addr)
            {
                start_addr += game_image_base;
                uptr addr = start_addr;
                auto disasm = hde::disasm_buffer_ex((void *)start_addr, MAX_INSTRUCTION_LEN * 20);
                if (!disasm.empty())
                {
                    for (auto it = disasm.begin(); it != disasm.end(); ++it)
                    {
                        const auto &ins = *it;
                        const hde64s *pnext_ins = nullptr;

                        if (it + 1 != disasm.end())
                        {
                            const auto &next_ins = *(it + 1);
                            pnext_ins = &next_ins;
                        }

                        if (pnext_ins != nullptr)
                        {
                            if (ins.opcode == 0x0F && ins.opcode2 == 0x11)
                            {
                                auto rs = *reinterpret_cast<u32 *>(addr + 4);
                                return rs;
                            }
                        }

                        addr += ins.len;
                    }
                }
            }
            return 0;
        }

    }

    bool collect_offsets(Context *ctx, LoaderContext *lctx, uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap)
    {
        if(gctx->m_opts.dump_game_image && !gctx->m_opts.debug_sigs) return true;

        bool fix = false;
        uptr game_cs_base_remote = 0;

        if (!from_snap)
        {
            auto remote_shdr_text = gctx->m_game_secs.get_sec_hdr(".text");
            if (!remote_shdr_text.VirtualAddress)
            {
                safeLog_msg("failed to grab remote segment header for TEXT.");
                return false;
            }
            game_cs_base_remote = remote_shdr_text.VirtualAddress;
            fix = true;
        }

        //
        // Grab Offsets
        //
        gctx->m_offsets.iface_base_client = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote,
                                                          "base_client", "48 8B 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 89 0D ? ? ? ? FF 10 48 8B 0D", 3, 7, fix);

        //
        //  Interfaces & Variables
        //
        lctx->offsets.entity_list = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "entity_list", "48 8D 15 ? ? ? ? 48 C1 E0 05 C1 E9 10 39 4C 10 08 75 34", 3, 7, fix);
        lctx->offsets.signon_state = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "signon_state", "48 63 05 ? ? ? ? 4C 8D 0D ? ? ? ? 4D 8B 0C C1", 3, 7, fix);
        lctx->offsets.input_system = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "input_system", "48 8D 0D ? ? ? ? 85 FD 74 13 41 3B DE BA ? ? ? ? 41 0F 45 D7", 3, 7, fix);
        lctx->offsets.mp_gamemode = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "mp_gamemode", "48 8B 0D ? ? ? ? 49 8B D6 48 8B 01 FF 50 70", 3, 7, fix);
        lctx->offsets.global_vars = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "global_vars", "48 8B 05 ? ? ? ? F3 0F 58 70 ? 0F 28 C6", 3, 7, fix);
        lctx->offsets.game_movement = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "game_movement", "48 8B 05 ? ? ? ? FF 50 20 48 8B 0D ? ? ? ? 4C 8D 3D", 3, 7, fix);
        lctx->offsets.map_name = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "map_name", "4C 8D 3D ? ? ? ? 49 C7 C0 ? ? ? ? 0F 1F 44 00 ? 49 FF C0 43 80 3C 07", 3, 7, fix);
        lctx->offsets.mat_system_surface = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "mat_system_surface", "48 8B 0D ? ? ? ? 8B 96 ? ? ? ? 48 8B 01 FF 50 68", 3, 7, fix);
        lctx->offsets.view_render = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "view_render", "48 8B 1D ? ? ? ? 48 8B 48 60 B8 ? ? ? ? 48 8B 51 10 66 39 02 75 58 48 63 4A 3C B8 ? ? ? ? 48 03 CA 66 39", 3, 7, fix);
        lctx->offsets.client_state = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "client_state", "48 8D 05 ? ? ? ? 33 D2 48 69 C9 ? ? ? ? 48 03 C8", 3, 7, fix);
        lctx->offsets.client_mode = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "client_mode", "48 8B 0D ? ? ? ? 48 8B 01 FF 50 68 84 C0", 3, 7, fix);
        lctx->offsets.kwewed_render_ctx = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "kwewed_render_ctx", "48 89 05 ? ? ? ? 48 8D 1D ? ? ? ? 48 8D 05 ? ? ? ? 66 0F 7F 05", 3, 7, fix);
        lctx->offsets.rs_cvar_gamepad_checks_enabled = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "rs_cvar_gamepad_checks_enabled", "48 8B 05 ? ? ? ? 48 8B DA 48 8B F1 83 78", 3, 7, fix); 
        lctx->offsets.rs_gamepad_checks_last = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "rs_gamepad_checks_last", "F2 0F 11 0D ? ? ? ? 48 89 1D ? ? ? ? 89 1D ? ? ? ? 88 1D ? ? ? ? 48 89 1D ? ? ? ? 88 1D ? ? ? ? 48 83 C4", 4, 8, fix);
        lctx->offsets.tag_list = FIND_PTR_LAJR(game_cs_base, game_cs_len, game_image_base, game_cs_base_remote, "tag_list", "48 8D 05 ? ? ? ? 48 8B 44 C8 ? 48 8B CF 48 85 C0 48 0F 45 D0 E8", 3, 7, fix);

        lctx->offsets.vft_caves.generic_1 = FIND_PTR_LAJR(game_cs_base, game_cs_len,
            game_image_base, game_cs_base_remote, "vft_caves_generic_1", "48 8D 05 ? ? ? ? 48 8D 04 C8 C3", 3, 7, fix);

        lctx->offsets.client_class_head = FIND_PTR_LAJR(game_cs_base, game_cs_len,
            game_image_base, game_cs_base_remote, "client_class_head", "4C 8B 1D ? ? ? ? 4D 85 DB 74 1D 0F 1F 40 00 49 8B 4B 10 48 8B D3", 3, 7, fix);

        lctx->offsets.string_dict_user_names = FIND_PTR_LAJR(game_cs_base, game_cs_len,
            game_image_base, game_cs_base_remote, "string_dict_user_names", "48 8B 0D ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 8B 01 45 33 C0 FF 90", 3, 7, fix);

        //
        // Functions
        //
        lctx->offsets.fns.get_mem_alloc = FIND_PTR_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, 
                                                               "fns.get_mem_alloc", 
                                                               "E8 ? ? ? ? 48 89 05 ? ? ? ? 4C 8B 00 BA ? ? ? ? 48 8B C8 41 FF 50 08 4C 8B 05", 
                                                               1, 
                                                               5, 
                                                               fix);

        lctx->offsets.fns.mss_draw_filled_rect = FIND_PTR_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, "fns.mss_draw_filled_rect", "E8 ? ? ? ? 8B B4 24 ? ? ? ? 44 8B C5", 1, 5, fix);
        
        lctx->offsets.fns.math_ang2vec = FIND_OFF_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, "fns.math_ang2vec", 
                                                               "48 8B C4 48 81 EC ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 10 69 ? F3 0F 59 2D", fix);
        lctx->offsets.fns.math_vec2ang = FIND_OFF_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, "fns.math_vec2ang", 
                                                               "48 83 EC 68 F3 0F 10 59 ? F3 0F 10 11 0F 28 C3 0F 29 74 24 ? 0F 29 7C 24", fix);
        //
        // Dynamic Offsets
        //
        lctx->offsets.name_list = find::wrap("name_list", game_image_base, game_cs_base, game_cs_len, from_snap, find::name_list);
        lctx->offsets.player.view_offset = find::wrap("player::view_offset", game_image_base, game_cs_base, game_cs_len, from_snap, find::player_view_offset);
        lctx->offsets.player.view_angle = find::wrap("player::view_angle", game_image_base, game_cs_base, game_cs_len, from_snap, find::player_view_angle);

        //
        // Return Addresses
        //
        lctx->offsets.ret_addrs.mkrctx_unk1 = FIND_OFF_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, "ret_addrs.mkrctx_unk1", 
                                                               "FF 90 ? ? 00 00 48 ? ? 48 ? ? C6 05 ? ? ? ? 00 0F", fix);
        if(lctx->offsets.ret_addrs.mkrctx_unk1 != 0) lctx->offsets.ret_addrs.mkrctx_unk1 += 6;

        lctx->offsets.ret_addrs.create_move = FIND_OFF_LAJR(game_cs_base, game_cs_len,
                                                               game_image_base, game_cs_base_remote, "ret_addrs.create_move", 
                                                               "FF 15 ? ? ? ? 0F B7 F8 85 FF 0F 84 ? ? ? ? 44 8D 0C FD ? ? ? ? 33 C0 45 85 C9 74 3C 44 0F BE 45 ? 48 8D 55 89 B9", fix);
        if(lctx->offsets.ret_addrs.create_move != 0) lctx->offsets.ret_addrs.create_move += 6;

        lctx->offsets.ret_addrs.override_mouse = FIND_OFF_LAJR(game_cs_base, game_cs_len,
                                                            game_image_base, game_cs_base_remote, "ret_addrs.override_mouse",
                                                            "FF 90 08 01 ? ? F3 ? ? ? ? ? ? ? ? 84 C0", fix);
        if(lctx->offsets.ret_addrs.override_mouse != 0) lctx->offsets.ret_addrs.override_mouse += 6;

        return true;
    }
}
