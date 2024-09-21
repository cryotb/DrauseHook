#include "inc/include.h"

namespace core
{
#if defined(DECLARE_AS_DEBUG_BUILD)
    LLVM_NOOPT void test_render_apis(respawn::Mat_system_surface* mss) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        mss->draw_text( 2, 14, { 50, 50 }, rs::clr::white, "Hello, World!" );

        render::line( { 250, 250 }, {500, 500}, rs::clr::white );
        render::rect( {510, 510 }, {550, 550}, rs::clr::black );
        render::rect_filled( {560, 560}, {600, 600}, rs::clr::white );
    }
#endif

    LLVM_NOOPT void on_render(int mode) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        if (!hooks::inited_deferred)
        {
            hooks::init_deferred();
            hooks::inited_deferred = true;
        }

        auto mss = g.ix.mss;
        
        g.inputs.ui.tick_begin();
        render::mss = mss;
        if (!is_valid_session())
        {
            g.inputs.ui.tick_end();
            return;
        }

        if (mode == 2)
        {
            auto me = local_player();
            if (me.is_valid())
            {
                EC::tick(me);
                EC::render(mss);
                features::on_esp(mss, me);
            }

            menu::on_render(mss);
        }

        g.inputs.ui.tick_end();
    }

    void on_level_init(const char *map) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        auto map_hash = *reinterpret_cast<const u64 *>(map);
        if (map_hash != decrypt_constant(encrypt_constant(0x7962626f6c5f706d)))
        {
            /* in an actual game*/
            g.ix.mss->get_screen_size(g.screen_dim[0], g.screen_dim[1]);
            
            g.ingame = true;
        } else
        {
            g.inlobby = true;
        }
    }

    void on_level_shutdown() LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        g.ingame = false;
        g.inlobby = false;

        EC::reset();
        g.r5ac_debug_pane_draw_time = 0.f;
        features::anonymizer_reset();
    }

    LLVM_NOOPT bool is_firing_range() LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        for (int i = 0; i < RS_ENTITY_LIST_SPACE; i++)
        {
            auto field = &g.ent_list[i];
            if (!field->ptr)
                continue;

            auto ent_base = field->ptr;

            auto ent = Entity(ent_base);
            if (!ent.is_valid())
                continue;

            if (ent.is_dummie())
                return true;
        }

        return false;
    }
}
