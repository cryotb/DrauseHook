#include "inc/include.h"

__declspec(noinline) GameModes get_game_mode(Entity &me)
{
	auto mp_gamemode_base = *(u64 *)(gctx->game_base + gctx->offsets.mp_gamemode);
	if (mp_gamemode_base)
	{
		auto gamemode_base = *(u64 *)(mp_gamemode_base + 0x50);
		if (gamemode_base)
		{
			auto gamemode = *(u64 *)gamemode_base;
			if (gamemode == decrypt_constant(encrypt_constant(0x6c61766976727573)))
				return GameModes::survival;

			else if (gamemode == decrypt_constant(encrypt_constant(0x6c006d6465657266)))
			{
				if (me.squad_id() == -1)
					return GameModes::tdm;
				else
					return GameModes::gunrun;
			}
			else if (gamemode == decrypt_constant(encrypt_constant(0x6c6f72746e6f63)))
				return GameModes::control;
		}
	}

	return GameModes::invalid;
}

__attribute((noinline)) bool is_valid_session()
{
	//if(!g.ingame)
	//	return false;

	auto signon_state = *g.signon_state;
	if (signon_state != 8)
		return false;

	auto map_name_hash = *(u64 *)(g.ctx->game_base + g.ctx->offsets.map_name);
	if (map_name_hash == 0 || map_name_hash == decrypt_constant(encrypt_constant(0x7962626f6c5f706d)))
		return false;

	auto my_base = *(u64 *)(g.ctx->game_base + g.ctx->offsets.game_movement + 0x8);
	if (!my_base)
		return false;

	auto me = Entity(my_base);
	if (!me.is_valid() || !me.is_alive() || !me.is_player())
		return false;

	g.game_mode = get_game_mode(me);
	if (g.game_mode == GameModes::invalid)
		return false;

	return true;
}

/*
	If lenient is FALSE, stricter checks will be employed.
*/
Entity local_player(bool lenient)
{
	auto my_base = *(u64 *)(g.ctx->game_base + g.ctx->offsets.game_movement + 0x8);
	if (!my_base)
		return Entity(0);

	auto me = Entity(my_base);
	if (!me.is_valid())
		return Entity(0);

	if (!lenient && !me.is_alive())
		return Entity(0);

	return Entity(my_base);
}

namespace respawn
{
	bool init()
	{
		// ...

		return true;
	}

	void *get_mm_inst()
	{
		using fn_get_mm_inst = void *(__fastcall *)();
		auto get_mm_inst_fn = reinterpret_cast<fn_get_mm_inst>(gctx->game_base + gctx->offsets.fns.get_mem_alloc);

		auto rs = get_mm_inst_fn();
		return rs;
	}

	char *get_name_ptr(Entity &ent)
	{
		auto name_list = (uptr *)(gctx->game_base + gctx->offsets.name_list);
		auto name_idx = ent.name_index();
		if (name_idx > 0 && name_idx < 100)
		{
			auto name_ptr = (char *)(name_list[3 * ent.name_index() - 3]);
			return name_ptr;
		}
		return nullptr;
	}

	char* get_clantag_ptr(Entity& ent)
	{
		auto name_list = (uptr *)(gctx->game_base + gctx->offsets.tag_list);
		auto name_idx = ent.name_index();
		if (name_idx > 0 && name_idx < 100)
		{
			auto name_ptr = (char *)(name_list[3 * ent.name_index() - 3]);
			return name_ptr;
		}
		return nullptr;
	}

	bool get_name(Entity &ent, char *buf, size_t len)
	{
		auto name_ptr = get_name_ptr(ent);
		if (name_ptr)
		{
			if (strlen(name_ptr) > 0 && strlen(name_ptr) < (len - 1))
			{
				auto clantag_ptr = get_clantag_ptr(ent);
				if(clantag_ptr != 0)
				{
					// if ptr is NOT zero, we got a clan tag to add.
					strcpy(buf, "[");
					strcpy(buf + strlen(buf), clantag_ptr);
					strcpy(buf + strlen(buf), "] ");
				}

				strcpy(buf + strlen(buf), name_ptr);
				return true;
			}
		}
		return false;
	}

	LLVM_NOOPT bool w2s(Vec3 &vIn, Vec3 &vOut) LLVMT( LLVMOBF_INLINE_FP() )
	{
		auto pvMatrix = g.ix.view_render->get_view_matrix(0);

		if (pvMatrix == nullptr)
			return false;

		auto vMatrix = *pvMatrix;

		vOut.x = vMatrix.m[0][0] * vIn.x + vMatrix.m[0][1] * vIn.y + vMatrix.m[0][2] * vIn.z + vMatrix.m[0][3];
		vOut.y = vMatrix.m[1][0] * vIn.x + vMatrix.m[1][1] * vIn.y + vMatrix.m[1][2] * vIn.z + vMatrix.m[1][3];

		float w = vMatrix.m[3][0] * vIn.x + vMatrix.m[3][1] * vIn.y + vMatrix.m[3][2] * vIn.z + vMatrix.m[3][3];

		if (w < 0.01)
			return false;

		float invw = 1.0f / w;

		vOut.x *= invw;
		vOut.y *= invw;

		const auto Cl_width = g.screen_dim[0];
		const auto Cl_height = g.screen_dim[1];

		auto x = static_cast<float>(Cl_width) / 2.f;
		auto y = static_cast<float>(Cl_height) / 2.f;

		x += 0.5f * vOut.x * Cl_width + 0.5f;
		y -= 0.5f * vOut.y * Cl_height + 0.5f;

		vOut.x = x;
		vOut.y = y;

		return true;
	}

	bool setup_render_points(const matrix3x4 &coordination_frame, 
		const Vec3 &mins, const Vec3 &maxs, rs::math::render_points_t &out)
	{
		const int num_transformations = 8;

		Vec3 points[num_transformations] = {
			{mins.x, mins.y, mins.z},
			{mins.x, maxs.y, mins.z},
			{maxs.x, maxs.y, mins.z},
			{maxs.x, mins.y, mins.z},
			{mins.x, maxs.y, maxs.z},
			{mins.x, mins.y, maxs.z},
			{maxs.x, mins.y, maxs.z}};

		Vec3 transformed[num_transformations];

		for (int index = 0; index < num_transformations; index++)
		{
			rs::math::vec_trans(points[index], coordination_frame, transformed[index]);
		}

		Vec3 flb{}, brt{}, blb{}, frt{}, frb{}, brb{}, blt{}, flt{};

		if (!w2s(transformed[3], flb) || !w2s(transformed[5], brt) ||
			!w2s(transformed[0], blb) || !w2s(transformed[4], frt) ||
			!w2s(transformed[2], frb) || !w2s(transformed[1], brb) ||
			!w2s(transformed[6], blt) || !w2s(transformed[7], flt))
		{
			return false;
		}

		Vec3 position_array[num_transformations] = {flb, brt, blb, frt, frb, brb, blt, flt};

		float left = flb.x;
		float top = flb.y;
		float right = flb.x;
		float bottom = flb.y;

		for (auto &index : position_array)
		{
			if (left > index.x)
			{
				left = index.x;
			}

			if (right < index.x)
			{
				right = index.x;
			}

			if (top < index.y)
			{
				top = index.y;
			}

			if (bottom > index.y)
			{
				bottom = index.y;
			}
		}

		out.x = left;
		out.y = bottom;
		out.w = (right - left);
		out.h = (top - bottom);

		return true;
	}

	bool hijack_playlist_data(void *data)
	{
#if !defined(PROD)
		if (g.conf.override_controller_aa)
		{
			auto pdata = data;
			auto pstr_assist_adspull_disable = find_substr((char *)pdata, "aimassist_adspull_disabled");
			if (pstr_assist_adspull_disable)
			{
				auto toggle = find_substr(pstr_assist_adspull_disable, "1");
				if (!toggle)
				{
					toggle = find_substr(pstr_assist_adspull_disable, "0");
				}

				if (toggle)
				{
					if (toggle[0] == '1')
					{
						toggle[0] = '0';
					}
				}
			}

			auto pstr_assist_magnet_pc = find_substr((char *)pdata, "aimassist_magnet_pc");
			if (pstr_assist_magnet_pc)
			{
				auto value = find_substr(pstr_assist_magnet_pc, "0.");
				if (value)
				{
					if (g.conf.override_controller_aa_value == 100)
					{
						value[0] = '1';
						value[1] = '.';
						value[2] = '0';
					} else if(g.conf.override_controller_aa_value > 0)
					{
						value[0] = '0';
						value[1] = '.';
						int adjusted = g.conf.override_controller_aa_value / 10;
						value[2] = tools::integer2char(adjusted);
					}
				}
			}
		}
#endif

		return true;
	}
}
