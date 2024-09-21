#include "inc/include.h"

extern "C"
{
	uptr ghk_orig_mkrctx_unk1 = 0;
	uptr ghk_orig_on_receive_server_info = 0;
	uptr ghk_orig_on_dropped_by_server = 0;
	uptr ghk_orig_process_playlist_override = 0;
	uptr ghk_orig_override_mouse = 0;
}

bool hooks::inited_deferred;
u32 hooks::vft_disp;
u32 hooks::vft_cave_cap;
uptr hooks::vft_cave_begin;

bool hooks::init_early()
{
	LMsg_Short("[hooks] initializing early.");

	hooks::inited_deferred = false;
	hooks::vft_disp = 0;
	hooks::vft_cave_cap = 0xBD0;
	hooks::vft_cave_begin = gctx->game_base + gctx->offsets.vft_caves.generic_1;

	return true;
}

bool hooks::init()
{
	LMsg_Short("[hooks] initializing regular.");

#if defined(DECLARE_AS_DEBUG_BUILD)
	LMsg(256, "hooks::init() -> vft_cave_01 =  begin:%p, end:%p", vft_cave_begin, (vft_cave_begin + vft_cave_cap));
#endif

	const auto handle_client_state = [&]()
	{
		auto vftcnt = vft::calc_len( *(void**)g.ix.client_state ) + 1;
		auto vftlm = vft::cave_setup( vftcnt, vft_cave_begin, vft_cave_cap, vft_disp );

		g.vmts.client_state.setup(vftcnt, PTR_OF(g.ix.client_state), vftlm);
		g.vmts.client_state.place(HOOK_IDX_ON_DROPPED_BY_SERVER, tp_on_dropped_by_server);
		ghk_orig_on_dropped_by_server = g.vmts.client_state.original<uptr>(HOOK_IDX_ON_DROPPED_BY_SERVER);
		g.vmts.client_state.enable();

		return true;
	};

	const auto handle_client_state_vft3 = [&]()
	{
		auto vftcnt = vft::calc_len( *(void**)(BASE_OF(g.ix.client_state) + 0x10) ) + 1;
		auto vftlm = vft::cave_setup( vftcnt, vft_cave_begin, vft_cave_cap, vft_disp );

		g.vmts.client_state_vft3.setup(vftcnt, PTR_OF((BASE_OF(g.ix.client_state) + 0x10)), vftlm);

		g.vmts.client_state_vft3.place(HOOK_IDX_ON_RECEIVED_SERVER_INFO, tp_on_receive_server_info);
		ghk_orig_on_receive_server_info = g.vmts.client_state_vft3.original<uptr>(HOOK_IDX_ON_RECEIVED_SERVER_INFO);

#if !defined(PROD)
		g.vmts.client_state_vft3.place(HOOK_IDX_PROCESS_PLAYLIST_OVERRIDE, tp_process_playlist_override);
		ghk_orig_process_playlist_override = g.vmts.client_state_vft3.original<uptr>(HOOK_IDX_PROCESS_PLAYLIST_OVERRIDE);
#endif
		g.vmts.client_state_vft3.enable();

		return true;
	};

	const auto handle_kwewed_render_ctx = [&]()
	{
		auto vftcnt = vft::calc_len( *(void**)g.ix.mat_kwewed_render_ctx ) + 1;
		auto vftlm = vft::cave_setup( vftcnt, vft_cave_begin, vft_cave_cap, vft_disp );

		g.vmts.mat_kwewed_render_ctx.setup(vftcnt, PTR_OF(g.ix.mat_kwewed_render_ctx), vftlm);
		g.vmts.mat_kwewed_render_ctx.place(HOOK_IDX_MKRCTX_UNK1, tp_mkrctx_unk1);
		ghk_orig_mkrctx_unk1 = g.vmts.mat_kwewed_render_ctx.original<uptr>(HOOK_IDX_MKRCTX_UNK1);
		g.vmts.mat_kwewed_render_ctx.enable();

		return true;
	};

	if(!handle_client_state() || !handle_client_state_vft3())
		return false;

	if(!handle_kwewed_render_ctx())
		return false;

	return true;
}

bool hooks::init_deferred()
{
	LMsg_Short("[hooks] initializing deferred.");

	const auto handle_client_mode = [&]()
	{
		auto pclient_mode = *g.ix.pclient_mode;

		auto vftcnt = vft::calc_len(*(void **)pclient_mode) + 1;
		auto vftlm = vft::cave_setup(vftcnt, vft_cave_begin, vft_cave_cap, vft_disp);

		g.vmts.client_mode.setup(vftcnt, PTR_OF(pclient_mode), vftlm);

		g.vmts.client_mode.place(HOOK_IDX_OVERRIDE_MOUSE, tp_override_mouse);
		ghk_orig_override_mouse = g.vmts.client_mode.original<uptr>(HOOK_IDX_OVERRIDE_MOUSE);

		g.vmts.client_mode.enable();

		return true;
	};

	if(!handle_client_mode())
		return false;

	return true;
}

unsigned short __fastcall hk_capture_stack_back_trace(unsigned long frames_to_skip, unsigned long frames_to_capture, void **trace, void *trace_hash)
{
	auto retaddr = BASE_OF(_ReturnAddress()) - gctx->game_base;
	g.r5ac_stack_walk_last_call = retaddr;

	if(g.conf.anon_mode)
	{
		*reinterpret_cast<uptr*>(gctx->game_base + gctx->offsets.string_dict_user_names) = 0;
	}

	if (retaddr == gctx->offsets.ret_addrs.create_move)
	{
		g.inputs.cmove.tick_begin();

		features::on_anonymizer();
		if (is_valid_session())
		{
			auto me = local_player();
			if (me.is_alive())
			{
				if (g.conf.rcs_strength > 0)
				{
					float rcs_strength = (static_cast<float>(g.conf.rcs_strength) / 1000.f);
					g.conf.rcs_internal_mouse_scale = rcs_strength;
				}

				if (g.conf.aim_assist_strength > 0)
				{
					float aa_strength = (static_cast<float>(g.conf.aim_assist_strength) / 100.f);
					g.conf.aim_assist_internal_mouse_scale = (gfloats.one - aa_strength);
				}

				features::on_movement(me);
				features::on_trigger(me);
			}
		}

		g.inputs.cmove.tick_end();
	}

	return 0; /* no traces are available */
}

void __fastcall hk_mkrctx_unk1(void* inst, u64 a2, void* retaddr)
{
	retaddr = reinterpret_cast<void*>( BASE_OF(retaddr) - gctx->game_base );
	if(retaddr == PTR_OF(gctx->offsets.ret_addrs.mkrctx_unk1))
	{
		core::on_render(2);
	}
}

void __fastcall hk_on_receive_server_info(void* inst, uptr netmsg)
{
	auto mapname = *(const char**)(netmsg + 0x60);

#if defined(DECLARE_AS_DEBUG_BUILD)
	LMsg(128, "recv_server_info: %s", mapname);
#endif

	core::on_level_init( mapname );
}

/* actually gets called also when DCing manually and because of other shit. */
void __fastcall hk_on_dropped_by_server(void* this_ptr, char unk_cond_1, const char* reason)
{
#if defined(DECLARE_AS_DEBUG_BUILD)
	LMsg(128, "disconnect: %s", reason);
#endif

	core::on_level_shutdown(  );

	strcpy(g.last_disconnect_reason, reason);

	if(reason[0] == '#')
	{
		if(strstr(reason, "ANTICHEAT_BANNED"))
		{
		#if LOGGING_IS_ENABLED == 1
			msg("client has been exempt from server, account banned.");
		#endif
			stl::fps::sleep(20);
			__fastfail(decrypt_constant(encrypt_constant(0xD1A122)));
		}
	}
}

void __fastcall hk_process_playlist_override(void* inst, void* net_msg)
{
#if !defined(PROD)
	char* data = reinterpret_cast<char*>(BASE_OF(net_msg) + 0x21);
	respawn::hijack_playlist_data(data);
#endif
}

void __fastcall hk_override_mouse(void *inst, void *retaddr)
{
	auto retaddr_rva = (BASE_OF(retaddr) - gctx->game_base);

	if (retaddr_rva == gctx->offsets.ret_addrs.override_mouse)
	{
		if (is_valid_session())
		{
			auto me = local_player();
			if (me.is_valid())
			{
				float mouse_x = 0.f, mouse_y = 0.f;
				float orig_mouse_x = 0.f, orig_mouse_y = 0.f;

				read_xmm7(&mouse_x);
				read_xmm8(&mouse_y);

				orig_mouse_x = mouse_x;
				orig_mouse_y = mouse_y;

				features::on_rcs(me, mouse_x, mouse_y);
				features::on_aim_assist(me, mouse_x, mouse_y);

				if (orig_mouse_x != mouse_x)
					write_xmm7(mouse_x);
				if (orig_mouse_y != mouse_y)
					write_xmm8(mouse_y);
			}
		}
	}
}
