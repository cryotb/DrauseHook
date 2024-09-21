#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)

#define VEC_EMPTY \
	Vec3 { 0.f, 0.f, 0.f }

namespace math
{
	inline static u32 _FOPI = float2dword(1.27323954473516);

	inline static u32 _DP1 = float2dword(0.78515625);
	inline static u32 _DP2 = float2dword(2.4187564849853515625e-4);
	inline static u32 _DP3 = float2dword(3.77489497744594108e-8);
	inline static u32 _lossth = float2dword(8192.);
	inline static u32 _T24M1 = float2dword(16777215.);

#define FOPI dword2float(_FOPI)
#define DP1 dword2float(_DP1)
#define DP2 dword2float(_DP2)
#define DP3 dword2float(_DP3)
#define lossth dword2float(_lossth)
#define T24M1 dword2float(_T24M1)

	extern float remainderf(float x, float y);

	LLVM_NOOPT inline float sqrtf(float _X) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_X))); }

	inline float acosf(float x) { return stl::Xacosf(x); }
	inline float atan2f(float y, float x) { return stl::Xatan2f(y, x); }
	inline float fabsf(float val) { return stl::Xfabsf(val); }
}

#pragma warning(pop)

struct global_float_constants
{
	float threesixty;
	float minus89;
	float plus89;
	float one;
	float point_one;
	float point_o5;
	float point_o1;
	float point_o2;
	float respawn_sens;

	void init()
	{
		_fp(threesixty, 360.f);
		_fp(minus89, -89.f);
		_fp(plus89, +89.f);
		_fp(one, 1.f);
		_fp(point_one, 0.1f);
		_fp(point_o5, 0.05f);
		_fp(point_o1, 0.01f);
		_fp(point_o2, 0.02f);
		_fp(respawn_sens, 0.022f);

		this->threesixty = threesixty;
		this->minus89 = minus89;
		this->plus89 = plus89;
		this->one = one;
		this->point_one = point_one;
		this->point_o5 = point_o5;
		this->point_o1 = point_o1;
		this->point_o2 = point_o2;
		this->respawn_sens = respawn_sens;
	}
} inline gfloats;

struct Point2
{
	Point2(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	Point2(float x, float y)
	{
		// WARNING: truncates everything except the decimal before comma!
		this->x = (int)x;
		this->y = (int)y;
	}

	int x;
	int y;
};

class Vec3
{
public:
	float x;
	float y;
	float z;

public:
	Vec3() = default;
	Vec3(float x_, float y_, float z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}

	float *base()
	{
		return reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(this));
	}

	auto base_ro() const
	{
		return reinterpret_cast<const float * const>(reinterpret_cast<uintptr_t>(this));
	}

	float &operator[](int index)
	{
		if (index == 0)
			return x;
		else if (index == 1)
			return y;
		else
			return z;
	}

	bool operator==(const Vec3 &other) const
	{
		return (other.x == x && other.y == y && other.z == z);
	}

	Vec3 operator-(const Vec3 &other) const
	{
		return Vec3{this->x - other.x, this->y - other.y, this->z - other.z};
	}

	Vec3 operator+(const Vec3 &other) const
	{
		return Vec3{this->x + other.x, this->y + other.y, this->z + other.z};
	}

	void operator+=(const Vec3 &other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
	}

	Vec3 &operator*=(const float multiplier)
	{
		x *= multiplier;
		y *= multiplier;
		z *= multiplier;

		return *this;
	}

	Vec3 operator*(const Vec3 &other) const { return {x * other.x, y * other.y, z * other.z}; }
	Vec3 operator*(const float &other) const { return {x * other, y * other, z * other}; }

	[[nodiscard]] float Length() const { return (x + y + z); }

	[[nodiscard]] float Length2d() const { return math::sqrtf(x * x + y * y + z * z); }
	[[nodiscard]] float LengthSquare() const { return (x * x + y * y + z * z); }
	[[nodiscard]] bool Empty() const { return (Length() <= 0.0f); }

	[[nodiscard]] float dist_to(const Vec3 &other)
	{
		auto delta = (other - *this);

		return math::fabsf(delta.LengthSquare());
	}
};

class Vec4 : public Vec3
{
public:
	float w;

public:
	Vec4() = default;
	Vec4(float x_, float y_, float z_, float w_)
	{
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}
};

class __declspec(align(16)) Vec4A : public Vec4
{
public:
	void Zero()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;
	}
};

namespace math
{
	namespace external
	{
		struct funs
		{
			void* ang2vec;
			void* vec2ang;
		} inline pfns;

		extern void init();
	}

	inline float dot_prod(const float *first, const float *second)
	{
		return ((first[0] * second[0]) + (first[1] * second[1]) + (first[2] * second[2]));
	}

	inline float dot_prod_vec(const Vec3 &first, const Vec3 &second)
	{
		return ((first.x * second.x) + (first.y * second.y) + (first.z * second.z));
	}

	LLVM_NOOPT inline float deg2rad(const float v)  LLVMT( LLVMOBF_INLINE_FP() )
	{
		return (v * static_cast<float>(M_PI)) / 180.f;
	}

	LLVM_NOOPT inline float rad2deg(const float v)  LLVMT( LLVMOBF_INLINE_FP() )
	{
		return (v * 180.0f) / static_cast<float>(M_PI);
	}

	LLVM_NOOPT inline void ang2vec(const Vec3 &in, Vec3 &out) LLVMT( LLVMOBF_INLINE_FP() )
	{
		using fun_ang2vec_t = void(__fastcall *)(const Vec3*, Vec3*);
		auto pfunc = reinterpret_cast<fun_ang2vec_t>(external::pfns.ang2vec);
		if(pfunc)
		{
			pfunc( &in, &out );
		}
	}

	LLVM_NOOPT inline void vec2ang(const Vec3 &forward, Vec3 &angles)  LLVMT( LLVMOBF_INLINE_FP() )
	{
		using fun_vec2ang_t = void(__fastcall *)(const Vec3*, Vec3*);
		auto pfunc = reinterpret_cast<fun_vec2ang_t>(external::pfns.vec2ang);
		if(pfunc)
		{
			pfunc( &forward, &angles );
		}
	}

	LLVM_NOOPT inline float get_fov_difference(Vec3 angle, Vec3 start, Vec3 end)  LLVMT( LLVMOBF_INLINE_FP() )
	{
		auto aim_angle = VEC_EMPTY;
		auto view_position = VEC_EMPTY, aim_position = VEC_EMPTY;

		auto delta = end - start;

		vec2ang(delta, aim_angle);

		ang2vec(angle, view_position);
		ang2vec(aim_angle, aim_position);

		const float aimplsq = aim_position.Length2d();
		const auto dotp = static_cast<float>(dot_prod_vec(aim_position, view_position) / aimplsq);

		const float vcos = acosf(dotp);
		const float drad = math::rad2deg(vcos);

		if (!__builtin_isnan(vcos))
			return drad;

		return 0.0f;
	}

	LLVM_NOOPT inline void normalize_angle(Vec3 &angle)  LLVMT( LLVMOBF_INLINE_FP() )
	{
		int i;

		for (i = 0; i < 3; i++)
		{
			if (angle[i] > 180.0)
			{
				angle[i] -= 360.0;
			}
			else if (angle[i] < -180.0)
			{
				angle[i] += 360.0;
			}
		}
	}
}
