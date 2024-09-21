#pragma once

//#define _RI_ENABLE_LOGGING

#include <intrin.h>
#include <xmmintrin.h>
#include <stdint.h>
#include <new>

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#include "defs.h"
#include "ntdefs.h"
#include "context.h"
#include "stl_base.h"
#include "rotate.h"
#include "string-table.h"
#include "xorstr.h"
#include "logger.h"
#include "loader.h"
#include "r5ac.h"

extern "C"
{
	extern LoaderContext* gctx;
}

#include "vfunc.h"
#include "xmmreg.h"
#include "build.h"
#include "stl.h"
#include "math.h"
#include "respawn.h"
#include "tools.h"
#include "vft.h"
#include "vmthk.h"
#include "input.h"

#pragma pack(push, 1)
struct Globals
{
	LoaderContext* ctx;
	u64 local_player_base;
	char hopping;
	char force_attack;
	int tea_bag_mode;
	entity_info_t* ent_list;
	GlobalVars* gvars;
	int* signon_state;
	GameModes game_mode;
	int screen_dim[2];
	bool ingame;
	bool inlobby;
	bool startup_debug_pane_finished;
	float startup_debug_pane_draw_time;
	bool trigger_reset_attack;

	float r5ac_debug_pane_draw_time;
	u32 r5ac_stack_walk_last_call;
	u32 r5ac_num_stack_walks_disarmed;

	char last_disconnect_reason[256];

	struct
	{
	} funcs;

	struct
	{
		u64 base_client;
		u64 client_state;
		u64* pclient_mode;
		u64 mat_kwewed_render_ctx;
		respawn::Mat_system_surface* mss;
		rs::Mem_alloc* mm;
		respawn::View_render* view_render;
	} ix;

	struct
	{
		int* rs_controller_checks_enabled;
		double* rs_controller_checks_last;
	} gv;

	struct
	{
		vmthk client_state;
		vmthk client_state_vft3;
		vmthk mat_kwewed_render_ctx;
		vmthk client_mode;
	} vmts;

	struct
	{
		Input ui;
		Input cmove;
	} inputs;

	struct
	{
		rs::In_state duck;
		rs::In_state jump;
		rs::In_state attack;
	} in_states;

	struct
	{
		bool visual;
		bool visual_far;
		bool visual_outline;

		bool rcs;
		int rcs_mode;
		int rcs_strength;
		float rcs_internal_mouse_scale;

		bool aim_assist;
		bool aim_assist_slow;
		bool aim_assist_rotational;
		int aim_assist_mode;
		int aim_assist_strength;
		float aim_assist_fov;
		float aim_assist_dead_zone;
		float aim_assist_internal_mouse_scale; // for slow
		float aim_assist_strength_ror;

		bool override_controller_aa;
		int override_controller_aa_value;

		bool anon_mode;

		bool trigger;

		LLVM_NOOPT void init() LLVMT(LLVMOBF_INLINE_FP())
		{
			visual = true;
			visual_far = false;
			rcs = false;
			anon_mode = false;
			rcs_mode = 1;
			rcs_strength = 25;
			override_controller_aa = false;
			override_controller_aa_value = 100;

			aim_assist = false;
			aim_assist_strength = 20;
			aim_assist_strength_ror = 0.034f;
			aim_assist_slow = true;
			aim_assist_rotational = true;
			aim_assist_dead_zone = 0.2f;
			aim_assist_fov = 9.25f;
			aim_assist_mode = 1;
		}
	} conf;
};
#pragma pack(pop)

extern Globals g;

#include "hooks.h"
#include "features.h"
#include "menu.h"
#include "core.h"
#include "renderer.h"
#include "entcache.h"
