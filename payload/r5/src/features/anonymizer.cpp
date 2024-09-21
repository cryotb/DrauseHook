#include "../inc/include.h"

namespace  /* ANONYMOUS */
{
    float time_last_update = 0.f;
}

namespace features
{
    LLVM_NOOPT void anonymize_player(Entity& ent) LLVMT(LLVMOBF_COMBO_STANDARD)
    {
        auto name_ptr = respawn::get_name_ptr(ent);
        if(name_ptr != 0)
        {
            name_ptr[0] = '\0';
        }

        auto clantag_ptr = respawn::get_clantag_ptr(ent);
        if(clantag_ptr != 0)
        {
            clantag_ptr[0] = '\0';
        }
    }

    LLVM_NOOPT void on_anonymizer() LLVMT(LLVMOBF_INLINE_FP(),LLVMOBF_COMBO_STANDARD)
    {
        if(!g.conf.anon_mode)
            return;
            
        if (!time_last_update || (Plat_FloatTime() - time_last_update) > 1.f)
        {
            for (int i = 0; i <= RS_MAX_PLAYERS; i++)
            {
                auto field = &g.ent_list[i];
                if (!field->ptr)
                    continue;

                auto ent_base = field->ptr;
                auto ent = Entity(ent_base);

                if (ent.is_valid() && ent.is_player())
                {
                    anonymize_player(ent);
                }
            }
            time_last_update = Plat_FloatTime();
        }
    }

    LLVM_NOOPT void anonymizer_reset()
    {
        time_last_update = 0.f;
    }
}