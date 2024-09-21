#pragma once

namespace nv::collision
{
	NETVAR_OFFSET_RECEIVER(vec_mins);
	NETVAR_OFFSET_RECEIVER(vec_maxs);

	extern void init();
}

class Collision
{
public:
    DATA_FIELD_DIRECT(vec_mins, Vec3, nv::collision::vec_mins);
    DATA_FIELD_DIRECT(vec_maxs, Vec3, nv::collision::vec_maxs);
};
