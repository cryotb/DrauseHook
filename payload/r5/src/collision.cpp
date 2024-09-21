#include "inc/include.h"

void nv::collision::init()
{
    NETVAR_ASSIGN(&vec_mins, "DT_CollisionProperty", "m_vecMins");
    NETVAR_ASSIGN(&vec_maxs, "DT_CollisionProperty", "m_vecMaxs");
}


