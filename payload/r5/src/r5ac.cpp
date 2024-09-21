#include "inc/include.h"

namespace r5::ac
{
    bool disarm_stack_walker()
    {
        size_t num_handled = 0;
        uint64_t needle = gctx->winapis.capture_stack_back_trace;
        if (needle)
        {
            uintptr_t offset = 0;
            auto data = (u8*)gctx->game_data_base;
            auto end = (u8*)(gctx->game_data_base + gctx->game_data_len);

            while (true)
            {
                auto result = stl::search(data + offset, end, (uint8_t *)&needle, sizeof(uintptr_t));
                if (result != 0)
                {
                    #if LOGGING_IS_ENABLED == 1
                        LMsg(256, "[R5AC] disarming stack walk at index %i and pointer %p", num_handled, result);
                    #endif
                    *reinterpret_cast<void **>(result) = hk_capture_stack_back_trace;
                    ++num_handled;
                    offset = result - (uintptr_t)data + 1;
                }
                else
                {
                    break; // No more occurrences found
                }
            }
        }

        g.r5ac_num_stack_walks_disarmed = num_handled;

        return (num_handled > 0);
    }
}
