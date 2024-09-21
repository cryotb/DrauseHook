#include "inc/include.h"

#define MENU_FONT_HEIGHT 14
#define MENU_FONT_HEIGHT_ADD (MENU_FONT_HEIGHT+2)

#define STARTUP_DEBUG_PANE_MAX_TIME 45.f
#define R5AC_DEBUG_PANE_MAX_TIME 45.f

namespace menu
{
	state_t state;
	respawn::Mat_system_surface *gmss = nullptr;

	void checkbox(int &idx, const char *text, bool &pval) LLVMT(LLVMOBF_COMBO_STANDARD)
	{
		auto clr = pval ? LColor(255, 255, 65) : LColor(120, 120, 120);
		gmss->draw_text(2, MENU_FONT_HEIGHT,
						{state.render_pos_x, state.render_pos_y + state.height}, clr, text);

		if (state.idx == idx)
		{
			if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_LEFT))
			{
				pval = false;
			}
			else if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_RIGHT))
			{
				pval = true;
			}
		}

		state.nitems++;
		state.height += MENU_FONT_HEIGHT_ADD;
		idx++;
	}

	void slider_options(int &idx, const char *text, int &pval, int valmax, char **opts) LLVMT(LLVMOBF_COMBO_STANDARD)
	{
		auto clr = pval > 0 ? LColor(255, 255, 65) : LColor(120, 120, 120);
		gmss->draw_text(2, MENU_FONT_HEIGHT, {state.render_pos_x, state.render_pos_y + state.height}, clr, _XS("%s [%s]"), text, opts[pval]);

		if (state.idx == idx)
		{
			if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_LEFT))
			{
				if (pval > 0)
					pval--;
			}
			else if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_RIGHT))
			{
				if (pval < valmax)
					pval++;
			}
		}

		state.nitems++;
		state.height += MENU_FONT_HEIGHT_ADD;
		idx++;
	}

	LLVM_NOOPT void slider_percentage(int &idx, const char *text, int &pval, int step) LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
	{
		auto clr = pval > 0.f ? LColor(255, 255, 65) : LColor(120, 120, 120);
		gmss->draw_text(2, MENU_FONT_HEIGHT, {state.render_pos_x, state.render_pos_y + state.height}, clr, _XS("%s (%i%%)"), text, pval);

		if (state.idx == idx)
		{
			if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_LEFT))
			{
				if (pval > 0)
					pval -= step;

				/* in case STEP made us negative. */
				if (pval < 0)
					pval = 0;
			}
			else if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_RIGHT))
			{
				if (pval < 100)
					pval += step;
			}
		}

		state.nitems++;
		state.height += MENU_FONT_HEIGHT_ADD;
		idx++;
	}

	LLVM_NOOPT void slider_float(int &idx, const char *text, float &pval, float maxval, float step, int precision = 0) LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
	{
		char fmt[64];
		memset(fmt, 0, sizeof(fmt));
		switch(precision)
		{
			case 1: strcpy(fmt, hash2str(0x37300878)); break;
			case 2: strcpy(fmt, hash2str(0xbd12c0e3)); break;
			case 3: strcpy(fmt, hash2str(0x665770b8)); break;
			case 4: strcpy(fmt, hash2str(0xe45c4108)); break;
			default: strcpy(fmt, hash2str(0x199d07b)); break;
		}


		auto clr = pval > 0.f ? LColor(255, 255, 65) : LColor(120, 120, 120);
		gmss->draw_text(2, MENU_FONT_HEIGHT, {state.render_pos_x, state.render_pos_y + state.height}, clr, fmt, text, pval);

		if (state.idx == idx)
		{
			if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_LEFT))
			{
				if (pval > 0.f)
					pval -= step;

				/* in case STEP made us negative. */
				if (pval < 0.f)
					pval = 0.f;
			}
			else if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_RIGHT))
			{
				if (pval < maxval)
					pval += step;
			}
		}

		state.nitems++;
		state.height += MENU_FONT_HEIGHT_ADD;
		idx++;
	}

	void on_update_pos()
	{
		state.render_pos_x = state.pos_x + 4;
		state.render_pos_y = state.pos_y + 4;
	}

	void init() LLVMT(LLVMOBF_COMBO_STANDARD)
	{
		memset(&state, 0, sizeof(state));

		state.visible = true;
		state.pos_w = 115;
		state.pos_h = 2;
		state.height = 0;
		state.pos_x = g.screen_dim[0] - state.pos_w - 2;
		state.pos_y = (g.screen_dim[1] / 12) * 5;

		on_update_pos();
	}

	LLVM_NOOPT void startup_debug_pane() LLVMT(LLVMOBF_INLINE_FP())
	{
#if !defined(PROD) && defined(DECLARE_AS_DEBUG_BUILD)
		if(!menu::is_visible() && g.startup_debug_pane_finished)
			return;
		
		if(!g.startup_debug_pane_draw_time)
		{
			if(Plat_FloatTime() > 0.1f)
			{
				g.startup_debug_pane_draw_time = Plat_FloatTime();
			}
		} else
		{
			if(!menu::is_visible() && (Plat_FloatTime() - g.startup_debug_pane_draw_time) > STARTUP_DEBUG_PANE_MAX_TIME)
			{
				g.startup_debug_pane_finished = true;
				return;
			}
		}

		int push = 0;

		int start_x = (g.screen_dim[0] / 2) - 200, start_y = 25;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::blue, "Startup!" );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, 
			"WINAPI_RtlCaptureStackBackTrace: %p", gctx->winapis.capture_stack_back_trace );
		push += 15;


#if defined(DECLARE_AS_DEBUG_BUILD)
		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, 
			"GAME_SCN_TEXT: %llx [L%llx]", gctx->game_cs_base, gctx->game_cs_len );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, 
			"GAME_SCN_RDATA: %llx [L%llx]", gctx->game_rdata_base, gctx->game_rdata_len );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, 
			"GAME_SCN_DATA: %llx [L%llx]", gctx->game_data_base, gctx->game_data_len );
		push += 15;
#endif

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, 
			"R5AC_NumStackWalksDisarmed: %i", g.r5ac_num_stack_walks_disarmed );
		push += 15;

		if (!menu::is_visible())
		{
			float time_left = STARTUP_DEBUG_PANE_MAX_TIME - (Plat_FloatTime() - g.startup_debug_pane_draw_time);
			gmss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "disappearing in %.1f", time_left);
			push += 15;
		}
#endif
	}

	LLVM_NOOPT void respawn_integrity_debug_pane() LLVMT(LLVMOBF_INLINE_FP())
	{
#if !defined(PROD) && defined(DECLARE_AS_DEBUG_BUILD)
		if(!g.r5ac_debug_pane_draw_time)
		{
			if(Plat_FloatTime() > 0.1f)
			{
				g.r5ac_debug_pane_draw_time = Plat_FloatTime();
			}
		} else
		{
			if( !menu::is_visible() && (Plat_FloatTime() - g.r5ac_debug_pane_draw_time) > R5AC_DEBUG_PANE_MAX_TIME )
			{
				return;
			}
		}

		int push = 0;

		int start_x = (g.screen_dim[0] / 2) + 200, start_y = 25;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, "RS_gamepad_checks: %i", *g.gv.rs_controller_checks_enabled );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, "RS_gamepad_checks_last: %f", *g.gv.rs_controller_checks_last );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, "R5_AC_swalk_lc: %x", g.r5ac_stack_walk_last_call );
		push += 15;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::white, "last_disconnect_reason: '%s'", g.last_disconnect_reason );
		push += 15;

#if defined(DECLARE_AS_DEBUG_BUILD)
		if(is_valid_session())
		{
			auto me = local_player();
			if (me.is_valid())
			{
				char name[RS_PLAYER_MAX_NAME_LEN];
                memset(name, 0, sizeof(name));

                respawn::get_name(me, name, sizeof(name));
				gmss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "local_player_name: '%s'", name);
				push += 15;
			}
		}
#endif

		if (!menu::is_visible())
		{
			float time_left = R5AC_DEBUG_PANE_MAX_TIME - (Plat_FloatTime() - g.r5ac_debug_pane_draw_time);
			gmss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "disappearing in %.1f", time_left);
			push += 15;
		}
#endif
	}

	LLVM_NOOPT void r5i_build_info_pane() LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
	{
		if(!menu::is_visible())
			return;

		int push = 0;
		int start_x = state.pos_x - 250, start_y = 5;

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::light_blue, "%s | %s", __DATE__, __TIME__ );
		push += 15;

#if defined(PROD)
		gmss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::light_blue, "Premium build, registered to %s", STRINGIFIED_DEF(_WATERMARKED_USER));
		push += 15;
#endif

		gmss->draw_text( 2, 14, { start_x, start_y + push }, rs::clr::light_blue, hash2str(0xffa91a45), STRINGIFIED_DEF(_COMPILED_AES_SEED_VALUE) );
		push += 15;
	}

	LLVM_NOOPT void on_render(respawn::Mat_system_surface *mss) LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
	{
		gmss = mss;

		if (!state.inited)
		{
			if(g.screen_dim[0] > 0 && g.screen_dim[1] > 0)
			{
				init();
				state.inited = true;
			} else
			{
				return;
			}
		}

		if(!g.conf.anon_mode)
		{
			respawn_integrity_debug_pane();
			startup_debug_pane();
		}
		
		r5i_build_info_pane();

		if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_NUMPAD_7))
		{
			state.visible = !state.visible;
		}

		if (!state.visible)
			return;

		if (is_valid_session())
		{
			/* only draw background when in game, because i
				struggle to read the values when in game due to diff terrain. */
			mss->draw_set_color(LColor(45, 45, 45));
			mss->draw_rect_filled(state.pos_x,
								  state.render_pos_y,
								  state.pos_x + state.pos_w,
								  state.render_pos_y + state.pos_h + state.height);
		}

		mss->draw_set_color(LColor(125, 125, 125, 155));
		mss->draw_rect_filled(state.pos_x,
							  state.render_pos_y + (state.idx * MENU_FONT_HEIGHT_ADD),
							  state.pos_x + state.pos_w,
							  state.render_pos_y + (state.idx * MENU_FONT_HEIGHT_ADD) + MENU_FONT_HEIGHT_ADD);

		if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_UP))
		{
			if (state.idx > 0)
				state.idx--;
		}
		else if (g.inputs.ui.was_button_pressed(rs::Buttons::KB_ARROW_DOWN))
		{
			if (state.idx < state.nitems - 1)
				state.idx++;
		}

		auto pos_x = state.render_pos_x;
		auto pos_y = state.render_pos_y;

		//
		//

		state.height = 0;
		state.nitems = 0;

		//
		//

		int idx = 0;

#if defined(DECLARE_AS_DEBUG_BUILD)
		gmss->draw_text(2, MENU_FONT_HEIGHT, {state.render_pos_x, state.render_pos_y + state.height}, 
			g.ingame ? rs::clr::green : rs::clr::white, _XS("IG: %i"), g.ingame);
		state.height += 15;
#endif

		checkbox(idx, hash2str(0xa4301260), g.conf.anon_mode);
		checkbox(idx, hash2str(0x75bc5d87), g.conf.visual);
		checkbox(idx, hash2str(0x3334d82a), g.conf.visual_far);
		checkbox(idx, hash2str(0xb1aef1bd), g.conf.visual_outline);

		char psz_rcs_modes[2][16];
		strcpy(psz_rcs_modes[0], hash2str(0x9d2228ca));
		strcpy(psz_rcs_modes[1], hash2str(0x69fb5606));

		char *ppsz_rcs_modes[2];
		ppsz_rcs_modes[0] = psz_rcs_modes[0];
		ppsz_rcs_modes[1] = psz_rcs_modes[1];
		checkbox(idx, hash2str(0x7e4b630f), g.conf.rcs);
		slider_options(idx, hash2str(0x36b16a7e), g.conf.rcs_mode, 1, ppsz_rcs_modes);
		slider_percentage(idx, hash2str(0xf319857d), g.conf.rcs_strength, 5);

		checkbox(idx, hash2str(0x6226f0e7), g.conf.aim_assist);
		slider_options(idx, hash2str(0x36b16a7e), g.conf.aim_assist_mode, 1, ppsz_rcs_modes);
		checkbox(idx, hash2str(0x1f7bc172), g.conf.aim_assist_slow);
		slider_percentage(idx, hash2str(0x385a66f4), g.conf.aim_assist_strength, 10);
		checkbox(idx, hash2str(0xd8321dbd), g.conf.aim_assist_rotational);
		slider_float(idx, hash2str(0x246f545c), g.conf.aim_assist_fov, 25.f, 0.25f, 2);
		slider_float(idx, hash2str(0xdebc8a9f), g.conf.aim_assist_dead_zone, 6.f, 0.1f, 1);
		slider_float(idx, hash2str(0xf319857d), g.conf.aim_assist_strength_ror, 0.2f, 0.002, 3);

#if !defined(PROD)
		checkbox(idx, _XS("RAO"), g.conf.override_controller_aa);
		slider_percentage(idx, _XS(" STR"), g.conf.override_controller_aa_value, 10);
#endif

		checkbox(idx, hash2str(0x6168876c), g.conf.trigger);
	}
}
