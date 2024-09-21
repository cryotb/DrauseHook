#include "../inc/include.h"

namespace /* ANONYMOUS */
{
#define SUPERGLIDE_STATE_IDLE 0
#define SUPERGLIDE_STATE_STARTED 1
    struct SuperGlideContext
    {
        u8 state;
        u8 force_duck;
        u8 force_reset;
    };

    SuperGlideContext sgctx{};
}

namespace features
{
    LLVM_NOOPT void bunnyhop(Entity &me, const rs::In_state &key_duck, const rs::In_state &key_jump) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        if (!g.hopping)
        {
            if (key_duck.active() && key_jump.active())
            {
                g.hopping = true;
            }
        }
        else
        {
            if (key_duck.state() != rs::InKeyStates::HOLDING)
            {
                g.hopping = false;
                return;
            }

            if (!(me.flags() & FL_ONGROUND))
            {
                key_jump.block();
                key_jump.reset();
            }
            else
            {
                key_jump.force();
            }
        }
    }

    LLVM_NOOPT bool super_glide(Entity &me, SuperGlideContext *ctx, const rs::In_state &key_duck, const rs::In_state &key_jump) LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
    {
        auto trav_progress = me.trav_progress();
        auto trav_start_time = me.trav_start_time();
        auto trav_release_time = me.trav_release_time();

        switch (ctx->state)
        {
        case SUPERGLIDE_STATE_IDLE:
        {
            if (trav_start_time >= g.gvars->Curtime)
                ctx->state = SUPERGLIDE_STATE_STARTED;

            break;
        }
        case SUPERGLIDE_STATE_STARTED:
        {
            if (ctx->force_duck)
            {
                key_duck.tap();
                key_jump.tap();
                ctx->force_duck = 0;
                ctx->force_reset = 1;
                return false;
            }

            if (ctx->force_reset)
            {
                ctx->force_reset = 0;
                ctx->state = SUPERGLIDE_STATE_IDLE;
                return true;
            }

            if (trav_progress >= 0.90f)
            {
                key_jump.tap();
                ctx->force_duck = 1;
            }

            break;
        }
        default:
            break;
        }

        return false;
    }

    void on_movement(Entity &me) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        if (super_glide(me, &sgctx, g.in_states.duck, g.in_states.jump))
        {
            g.in_states.duck.reset();
            g.in_states.jump.reset();
        }
        else
        {
            bunnyhop(me, g.in_states.duck, g.in_states.jump);
        }
    }
}
