#include "inc/include.h"

namespace rs::math
{
    bool screen_range_is_within_crosshair(int *begin, int *end)
    {
        int cross[2] = {(int)g.screen_dim[0] / 2, (int)g.screen_dim[1] / 2};

        return (cross[0] >= begin[0] && cross[0] < end[0]) &&
               (cross[1] >= begin[1] && cross[1] < end[1]);
    }

    bool screen_make_bubble_for_combat_ent(Entity &ent, int *obegin, int *oend, float scale) LLVMT(LLVMOBF_INLINE_FP())
    {
        if (scale < 0.01f)
            scale = 1.f;

        auto rec = EC::get(ent.eidx());
        if (!EC::is_cached_record_valid(ent, rec))
            return false;

        if(!rec->rpts_avail)
            return false;

        auto pos_origin = ent.origin();
        Vec3 scr_origin = VEC_EMPTY;
        if (!respawn::w2s(pos_origin, scr_origin))
            return false;

        int bounds_height = rec->rpts.h;
        if(!bounds_height) return false;
        int bounds_width = rec->rpts.w * scale;

        obegin[0] = (int)(scr_origin.x - bounds_width);
        obegin[1] = (int)(scr_origin.y - bounds_height);

        oend[0] = (int)(scr_origin.x + bounds_width);
        oend[1] = (int)(scr_origin.y + 5.f);

        return true;
    }
}

