#pragma once

typedef u32 ehandle;

#define RS_ENTITY_LIST_SPACE 0x10000
#define RS_EHANDLE_INVALID 0xFFFFFFFF
#define EHANDLE_IS_VALID(X) (X != RS_EHANDLE_INVALID)
#define RS_EHANDLE_TO_INDEX(h) \
	(!EHANDLE_IS_VALID(h) ? -1 : (int)(h & (u32)(RS_ENTITY_LIST_SPACE - 1)))

struct Color
{
	Color(float r_, float g_, float b_)
	{
		r = r_;
		g = g_;
		b = b_;
	}

	float r;
	float g;
	float b;
};

struct LColor
{
	LColor(int r, int g, int b, int a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	LColor(int r, int g, int b) : LColor(r, g, b, 255)
	{
	}

	LColor() = default;

	int r;
	int g;
	int b;
	int a;
};

namespace rs::clr
{
	inline LColor white;
	inline LColor black;
	inline LColor green;
	inline LColor blue;
	inline LColor light_blue;

	inline void init()
	{
		white = LColor(255, 255, 255);
		black = LColor(0, 0, 0);
		green = LColor(45, 255, 45);
		blue = LColor(45, 45, 255);
		light_blue = LColor(66, 135, 245);
	}
}

inline Vec4 to_vec4(Vec3 vec, float w)
{
	return Vec4(vec.x, vec.y, vec.z, w);
}

struct matrix3x4
{
	float *operator[](int i)
	{
		return m_value[i];
	}

	const float *operator[](int i) const
	{
		return m_value[i];
	}

	float *Base()
	{
		return &m_value[0][0];
	}

	const float *Base() const
	{
		return &m_value[0][0];
	}

	float m_value[3][4];
};

struct matrix4x4
{
	float *operator[](int i)
	{
		return m_value[i];
	}

	const float *operator[](int i) const
	{
		return m_value[i];
	}

	float *Base()
	{
		return &m_value[0][0];
	}

	const float *Base() const
	{
		return &m_value[0][0];
	}

	float m_value[4][4];
};

struct __declspec(align(16)) matrix3x4a : matrix3x4
{
};

#include "respawn/defs.h"
#include "respawn/prototypes.h"
#include "respawn/dt_recv.h"
#include "respawn/netvars.h"
#include "respawn/collision.h"
#include "respawn/entity.h"
#include "respawn/input_system_buttons.h"

#pragma pack(push, 1)
struct entity_info_t
{
	u64 ptr;
	n64 serial;
	u64 prev;
	u64 next;
};
#pragma pack(pop)

bool is_valid_session();
Entity local_player(bool lenient = false);

#define RS_WAS_BUTTON_DOWN(a2) (((*(u32 *)((g.ctx->game_base + gctx->offsets.input_system) + 4 * ((u64)(a2) >> 5) + 0xB0)) & (1 << ((a2)&0x1F))) != 0)

enum class Legends
{
	Invalid = -1,
	Unset,
	Octane,
	Gibby,
	Bang,
	Wraith,
	Pathy,
	Wifeline,
};

class GlobalVars
{
public:
	char pad_0000[16];	   // 0x0000
	float Curtime;		   // 0x0010
	char pad_0014[12];	   // 0x0014
	float Frametime;	   // 0x0020
	char pad_0024[12];	   // 0x0024
	float IntervalPerTick; // 0x0030
	int32_t MaxClients;	   // 0x0034
	char pad_0038[8];	   // 0x0038
	int32_t TickCount;	   // 0x0040
	char pad_0044[1052];   // 0x0044
};						   // Size: 0x0460
static_assert(sizeof(GlobalVars) == 0x460);

namespace d3d
{
	typedef struct _D3DMATRIX
	{
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};
	} D3DMATRIX;
}

#pragma pack(push, 1)
struct R5_file_info
{
    void* data;
    int32_t length;
};
#pragma pack(pop)

#include "respawn/math.h"
#include "respawn/in_state.h"
#include "respawn/iface/ifaces.h"

namespace respawn
{
	bool init();
	void *get_mm_inst();
	char *get_name_ptr(Entity &ent);
	bool get_name(Entity &ent, char *buf, size_t len);
	char* get_clantag_ptr(Entity& ent);
	bool w2s(Vec3 &vIn, Vec3 &vOut);
	bool hijack_playlist_data(void* data);
	bool setup_render_points(const matrix3x4 &coordination_frame, 
		const Vec3 &mins, const Vec3 &maxs, rs::math::render_points_t &out);
}
