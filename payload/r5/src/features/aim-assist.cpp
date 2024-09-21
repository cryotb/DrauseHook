#include "../inc/include.h"

namespace features
{
    LLVM_NOOPT Entity rotational_find_target(Entity &me,
                                                     Vec3 &outpos, float max_dist, float max_fov, float dead_zone, bool& should_slow) LLVMT( LLVMOBF_INLINE_FP(), LLVMOBF_COMBO_STANDARD )
    {
        constexpr float fov_treshold = 0.4f;

        auto my_origin = me.origin();
        auto my_shoot_pos = me.view_pos();
        auto my_ang = me.view_angle();
        auto target = Entity(0);

        float closest_fov = max_fov;

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
            if (!ent.is_alive())
                continue;
            if (ent.base() == me.base())
                continue;
            if (ent.is_ally_of(me))
                continue;

            auto rec = EC::get(ent.eidx());
            if (!EC::is_cached_record_valid(ent, rec))
                continue;
            
            if(!rec->is_visible)
                continue;

            auto aimpos = ent.get_center_pos();

            auto origin = ent.origin();
            float dist = math::sqrtf(origin.dist_to(my_origin)) / 100.f;
            auto fov = math::get_fov_difference(my_ang, my_shoot_pos, aimpos);

            if (dist > max_dist)
                continue;
            if (fov > max_fov)
                continue;

            if(dead_zone > 0.f)
            {
                int bubble_begin[2];
		        int bubble_end[2];

                memset(bubble_begin, 0, sizeof(bubble_begin));
                memset(bubble_end, 0, sizeof(bubble_end));

                if(!rs::math::screen_make_bubble_for_combat_ent(ent, bubble_begin, bubble_end, dead_zone))
                    continue;

                /* here we decide to stop the ROR-AA, because
                    we have a target that is in the dead zone. 
                    you do not want it to swap to another target,
                    because we already have one, so focus on that. */
                if(rs::math::screen_range_is_within_crosshair(bubble_begin, bubble_end))
                {
                    should_slow = true;
                    break;
                }
            }

            if (fov < closest_fov && (closest_fov - fov) >= fov_treshold)
            {
                target = ent;
                outpos = aimpos;
                closest_fov = fov;
            }
        }

        return target;
    }

    LLVM_NOOPT void rotational(Entity &me, float &x, float &y) LLVMT( LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD )
    {
        if (g.conf.aim_assist_mode == 0 && (x == 0.f && y == 0.f))
            return;

        if (me.is_sky_diving())
            return;

        auto my_ang = me.view_angle();
        auto my_shoot_pos = me.view_pos();
        auto target_pos = VEC_EMPTY;
        auto should_slow = false;
        auto target = rotational_find_target(me, target_pos,
                                             12.f, g.conf.aim_assist_fov, g.conf.aim_assist_dead_zone, should_slow);

        if(g.conf.aim_assist_slow && should_slow)
        {
            x = (x * g.conf.aim_assist_internal_mouse_scale);
            y = (y * g.conf.aim_assist_internal_mouse_scale);

            return;
        }

        if (target.is_valid())
        {
            Vec3 aim_ang = VEC_EMPTY;
            math::vec2ang((target_pos - my_shoot_pos), aim_ang);
            math::normalize_angle(aim_ang);

            Vec3 aim_delta = (my_ang - aim_ang);
            math::normalize_angle(aim_delta);

            aim_delta.x /= 0.022f;
            aim_delta.y /= 0.022f;

            auto vec_mouse = Vec3(x, y, 0.f);
            auto vec_delta = Vec3(aim_delta.y, -aim_delta.x, 0.f);
            vec_delta += vec_mouse;

            x += vec_delta.x * g.conf.aim_assist_strength_ror;
        }
    }

    LLVM_NOOPT void on_aim_assist(Entity &me, float &x, float &y)
    {
        if (!g.conf.aim_assist)
            return;

        if (g.conf.aim_assist_rotational)
            rotational(me, x, y);
    }
}
