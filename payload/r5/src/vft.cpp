#include "inc/include.h"

namespace vft
{
    void *cave_setup(size_t table_count, uptr cave_begin, u32 cave_cap, u32 &disp)
    {
        auto vftable_required_count = table_count + 2; /* plus acts as some sort of padding.*/
        auto vftable_required_length = (vftable_required_count * sizeof(uptr));
#if LOGGING_IS_ENABLED == 1
        LMsg(256, "vft_cave_setup:  required_count:%lld, required_length:0x%04X, disp: 0x%04X",
             vftable_required_count, vftable_required_length, disp);
#endif

        if (vftable_required_count < 1 || vftable_required_length < sizeof(uptr))
        {
#if LOGGING_IS_ENABLED == 1
            LMsg(256, "vft_cave_setup:  illegal vft characteristics!");
#endif

            __fastfail(_encoded_const(0xDFDB100));
        }

        auto space = reinterpret_cast<void *>(cave_begin + disp);

        uptr space_base = BASE_OF(space);
        uptr space_end = (space_base + vftable_required_length);
        uptr cave_end = (cave_begin + cave_cap);
        if (space_end >= cave_end)
        {
#if LOGGING_IS_ENABLED == 1
            LMsg(256, "vft_cave_setup:  no more space left in cave!");
#endif

            __fastfail(_encoded_const(0xDFDB102));
        }

        memset(space, 0, vftable_required_length);
        disp += vftable_required_length;

#if LOGGING_IS_ENABLED == 1
        LMsg(256, "vft_cave_setup:  returning cave space at %p (space left after this: %llx)", 
            space, cave_end - (BASE_OF(space) + vftable_required_length));
#endif

        return space;
    }

    size_t calc_len(void *ptbl)
    {
#if LOGGING_IS_ENABLED == 1
        LMsg(128, "calc_vft_len -> calculating for vftable at %p", ptbl);
#endif

        auto tbl = (uptr *)ptbl;
        size_t result = 0;

        while (true)
        {
            auto handler = tbl[result];

            if (!handler)
            {
#if LOGGING_IS_ENABLED == 1
                LMsg(128, "calc_vft_len -> abort: handler is zero.");
#endif
                break;
            }

            /* is the handler within game image at all? */
            if (handler < gctx->game_base || handler > (gctx->game_base + gctx->game_len))
            {
#if LOGGING_IS_ENABLED == 1
                LMsg(256, "calc_vft_len -> abort: handler %p is not in game image.", handler);
#endif
                break;
            }

            /* is the handler within .text section? */
            if (handler < gctx->game_cs_base || handler > (gctx->game_cs_base + gctx->game_cs_len))
            {
#if LOGGING_IS_ENABLED == 1
                LMsg(256, "calc_vft_len -> abort: handler %p is not in game code.", handler);
#endif
                break;
            }

            result++;
        }

        return result;
    }
}
