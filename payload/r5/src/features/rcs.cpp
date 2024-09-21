#include "../inc/include.h"

namespace features
{
	LLVM_NOOPT void on_rcs(Entity &me, float &x, float &y) LLVMT( LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD )
	{
		if (!g.conf.rcs)
			return;

		if(g.conf.rcs_mode == 0)
		{
			if(x == 0.f && y == 0.f)
				return;
		}

		auto recoil = me.punch_angle();
		auto my_ang = me.view_angle();
		auto aim_ang = my_ang - recoil;

		Vec3 aim_delta = (my_ang - aim_ang);

		aim_delta.x /= gfloats.respawn_sens;
		aim_delta.y /= gfloats.respawn_sens;

		auto vec_mouse = Vec3(x, y, 0.f);
		auto vec_delta = Vec3(aim_delta.y, -aim_delta.x, 0.f);
		vec_delta += vec_mouse;

		x += vec_delta.x * g.conf.rcs_internal_mouse_scale;
		y += vec_delta.y * g.conf.rcs_internal_mouse_scale;
	}
}
