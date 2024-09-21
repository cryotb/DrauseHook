#include "inc/include.h"

Globals g;
LoaderContext *gctx = (LoaderContext *)0x0AAAAAAAAAAAAAAAC;

LLVM_NOOPT bool init_early() LLVMT(LLVMOBF_COMBO_STANDARD)
{
	gfloats.init();
	rs::clr::init();

	auto ctx = gctx;
	memset(&g, 0, sizeof(g));
	g.ctx = ctx;

	if (!stl::init(ctx->libc_base, ctx->libm_base))
		return false;

	if(!r5::ac::disarm_stack_walker())
		return false;
		
	if(!hooks::init_early())
		return false;

	return true;
}

LLVM_NOINLINE bool init() LLVMT(LLVMOBF_COMBO_STANDARD)
{
	auto ctx = gctx;

	g.signon_state = (int *)(ctx->game_base + ctx->offsets.signon_state);
	g.ent_list = (entity_info_t *)(ctx->game_base + ctx->offsets.entity_list);
	g.gvars = *(GlobalVars **)(ctx->game_base + ctx->offsets.global_vars);
	g.gv.rs_controller_checks_enabled = reinterpret_cast<int*>( *(uptr*)(ctx->game_base + ctx->offsets.rs_cvar_gamepad_checks_enabled) + 0x64);
	g.gv.rs_controller_checks_last = reinterpret_cast<double*>( ctx->game_base + ctx->offsets.rs_gamepad_checks_last );

	g.ix.base_client = (ctx->game_base + ctx->offsets.base_client);
	g.ix.client_state = (uptr)(ctx->game_base + ctx->offsets.client_state);
	g.ix.mm = (rs::Mem_alloc *)(BASE_OF(respawn::get_mm_inst()));
	g.ix.mss = *(respawn::Mat_system_surface **)(ctx->game_base + ctx->offsets.mat_system_surface);
	g.ix.view_render = (respawn::View_render *)(ctx->game_base + ctx->offsets.view_render);
	g.ix.mat_kwewed_render_ctx = (uptr)(ctx->game_base + ctx->offsets.kwewed_render_ctx);
	g.ix.pclient_mode = reinterpret_cast<uptr *>(gctx->game_base + gctx->offsets.client_mode);

	// restore the VFT for init hook, this used to be the loaders job but we do it now.
	*reinterpret_cast<uptr *>(g.ix.base_client) = gctx->vft_orig_ptrs.base_client;

	if (!respawn::init())
		return false;
	math::external::init();

	g.inputs.ui.init();
	g.inputs.cmove.init();

	g.in_states.duck.construct( gctx->game_base + STATIC_OFFSET(0x73F2E00) );
	g.in_states.jump.construct( gctx->game_base + STATIC_OFFSET(0x73F2D00) );
	g.in_states.attack.construct( gctx->game_base + STATIC_OFFSET(0x73F2C20) );

	g.conf.init();

	*g.gv.rs_controller_checks_enabled = 0;

	if(!rs::netvars::init())
		return false;

	if(!EC::init())
		return false;

	if (!hooks::init())
		return false;

	ctx->inited = 1;

	return true;
}

extern "C"
{
	LLVM_NOOPT __attribute__((naked)) __declspec(dllexport) LLVM_NOINLINE void __fastcall _kb()
	{
		__asm
		{
			jmp tp_init
		}
	}

	LLVM_NOOPT LLVM_NOINLINE void main()
	{
		if (!init_early())
		{
			__fastfail(_encoded_const(0x1337));
		}

		if (!init())
		{
			__fastfail(_encoded_const(0x1227));
		}
	}
}
