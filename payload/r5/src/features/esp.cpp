#include "../inc/include.h"

namespace features
{
	LLVM_NOOPT void draw_outline(const rs::math::render_points_t &rp, const LColor &sColor) LLVMT( LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD )
	{
		render::rect({rp.x - 1.0f, rp.y - 1.0f}, {rp.x + rp.w + 1.0f, rp.y + rp.h + 1.0f}, LColor(5, 5, 5));
		render::rect({rp.x, rp.y}, {rp.x + rp.w, rp.y + rp.h}, sColor);
		render::rect({rp.x + 1.0f, rp.y + 1.0f}, {rp.x + rp.w - 1.0f, rp.y + rp.h - 1.0f}, LColor(5, 5, 5));
	}

	LLVM_NOOPT void draw_bar(const LColor& color, float flX, float flY, float flW, float flH, int current, const int max, bool bg = true) LLVMT( LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD )
	{
		if (current < 0)
			current = 0;

		if (current > max)
			current = max;

		flY -= flH;

		const float base = flH / static_cast<float>(max) * static_cast<float>(current);
		const float delta = (flH - base);

		const float bar_x = flX;

		if (bg)
			render::rect_filled({bar_x, flY}, {bar_x + 5.0f, flY + flH}, LColor(5, 5, 5));

		render::rect_filled({bar_x, flY + delta}, {bar_x + 5.0f, flY + flH}, color);

		if (bg)
			render::rect({bar_x, flY}, {bar_x + 5.0f, flY + flH}, LColor(5, 5, 5));
	}

	LLVM_NOOPT void draw_bar_vert(const LColor& color, float flX, float flY, float flW, float flH, int current, const int max, bool bg = true, bool outline = false) LLVMT( LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD )
	{
		if (current < 0)
			current = 0;

		if (current > max)
			current = max;

		flW *= 2.f;

		const float base = flW / static_cast<float>(max) * static_cast<float>(current);
		const float delta = (flW - base);

		const float bar_x = flX + 5.f;
		const float bar_y = flY + 7.f;
		const float height = 6.f;

		if (bg)
			render::rect_filled({bar_x, bar_y}, {bar_x + flW, bar_y + height}, LColor(5, 5, 5));

		render::rect_filled({bar_x, bar_y}, {bar_x + flW - delta, bar_y + height}, color);

		if (bg || outline)
			render::rect({bar_x, bar_y}, {bar_x + flW, bar_y + height}, LColor(5, 5, 5));
	}

	LLVM_NOOPT void process_player(respawn::Mat_system_surface *mss, Entity &me, Entity &pl) LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
	{
		auto rec = EC::get(pl.eidx());
		if(!EC::is_cached_record_valid(pl, rec))
			return;

		if (!rec->is_alive || rec->is_ally_of_local)
			return;

		if(!rec->rpts_avail)
			return;
		
		if(rec->rpts.x == 0 || rec->rpts.y == 0 ||
			rec->rpts.w == 0 || rec->rpts.h == 0)
			return;

		auto vec_origin_pos = rec->vec_origin[0];
		auto vec_origin_scr = rec->vec_origin[1];

		if (!rec->bscr_origin_avail)
			return;

		auto health = rec->health[0];
		auto armor = rec->armor[0];

		auto clr = rec->clr;
		auto rpts = rec->rpts;

		const auto draw_pos_top_x = rpts.x;
		const auto draw_pos_top_y = rpts.y - 23.f;
		int draw_pos_top_push = 0;

		auto name = rec->name;

		float dist = rec->dist_to_local;
		if (dist > 10000.f && !me.is_sky_diving())
		{
			if (g.conf.visual_far)
			{
				if (!g.conf.anon_mode)
				{
					mss->draw_text(2, 12, {vec_origin_scr.x, vec_origin_scr.y},
								   clr,
								   name);
				}

				mss->draw_text(2, 12, {vec_origin_scr.x, vec_origin_scr.y + 14},
							   clr,
							   _XS("[%02d]"), (int)dist);
			}

			return;
		}

		if (!g.conf.anon_mode)
		{
			mss->draw_text(2, 12, {draw_pos_top_x, draw_pos_top_y - draw_pos_top_push},
						   clr,
						   name);
			draw_pos_top_push += 15;
		}
		
		if (pl.is_player())
		{
			mss->draw_text(2, 12, {draw_pos_top_x, draw_pos_top_y - draw_pos_top_push},
						   rs::clr::white,
						   "LVL %i", rec->level);
			draw_pos_top_push += 15;
		}

		draw_bar(
			LColor(25, 158, 25), rpts.x - ((10.f / 2.f) + 2.f),
			rpts.y + rpts.h, rpts.w, rpts.h,
			health, rec->health[1], true);

		auto armor_max = rec->armor[1];
		if (armor_max != 0)
		{
			draw_bar_vert(
				LColor(82, 82, 242), rpts.x - 7.f, rpts.y - 15.f, rpts.w / 2.f, rpts.h,
				armor, armor_max, true, true);
		}

		if(g.conf.visual_outline)
		{
			draw_outline( rpts, clr );
		}
	}

	LLVM_NOOPT void on_esp(respawn::Mat_system_surface *mss, Entity &me) LLVMT( LLVMOBF_COMBO_STANDARD )
	{
		if (!g.conf.visual)
			return;

		for (int i = 0; i < RS_ENTITY_LIST_SPACE; i++)
		{
			auto field = &g.ent_list[i];
			if (!field->ptr)
				continue;

			auto ent_base = field->ptr;

			auto ent = Entity(ent_base);
			if (!ent.is_valid())
				continue;
			if (!ent.is_player() && !ent.is_dummie())
				continue;
			process_player(mss, me, ent);
		}
	}
}
