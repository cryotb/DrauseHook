#include "../inc/include.h"

namespace features
{
    LLVM_NOOPT void on_trigger(Entity &me) LLVMT(LLVMOBF_COMBO_STANDARD, LLVMOBF_INLINE_FP())
    {
        if(!g.conf.trigger)
            return;

        if(!g.inputs.cmove.is_button_down(rs::Buttons::KB_KEY_SHIFT))
            return;
            
        if(g.trigger_reset_attack)
        {
            g.in_states.attack.reset();
            g.in_states.attack.tap();

            g.trigger_reset_attack = false;
        }

        if (me.is_sky_diving())
            return;

        auto my_ang = me.view_angle();
        auto my_shoot_pos = me.view_pos();
        auto target_pos = VEC_EMPTY;
        auto should_slow = false;
        auto target = rotational_find_target(me, target_pos,
                                             1200.f, 20.f /* fov does not matter much */, 0.1f /* low dead zone for best accuracy */, should_slow);

        if(should_slow)
        {
            g.in_states.attack.force();
            g.trigger_reset_attack = true;
        }
    }
}
