#include "inc/include.h"

namespace math
{
    /*
     * ONLY USE FUNCTIONS WITH SIMPLE CONTROL FLOW 
     * FROM WITHIN THE GAME IMAGE SO THAT WE CAN CONTROL STACK WALKS.
    */
    void external::init()
    {
        memset(&pfns, 0, sizeof(funs));

        pfns.ang2vec = reinterpret_cast<void*>(gctx->game_base + gctx->offsets.fns.math_ang2vec);
        pfns.vec2ang = reinterpret_cast<void*>(gctx->game_base + gctx->offsets.fns.math_vec2ang);
    }
    
    __forceinline float remainderf(float x, float y)
    {
        int xi = *(int*)&x;
        int yi = *(int*)&y;
        int q = (int)(x / y);
        return x - (float)q * y;
    }
}
