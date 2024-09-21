/*
    Math HEADER dedicated to respawn stuff
    What does this one have that the main math HEADER doesnt:
        - can use respawn defs and structs.
    Keep that in mind when working on this.
*/

namespace rs::math
{
    struct render_points_t
    {
        float x;
        float y;
        float w;
        float h;
    };

    inline void vec_trans(Vec3 &in, const matrix3x4 &matrix, Vec3 &out)
	{
		const auto in_array = in.base();

		out.x = ::math::dot_prod(in_array, matrix[0]) + matrix[0][3];
		out.y = ::math::dot_prod(in_array, matrix[1]) + matrix[1][3];
		out.z = ::math::dot_prod(in_array, matrix[2]) + matrix[2][3];
	}

    LLVM_NOOPT extern bool screen_range_is_within_crosshair(int* begin, int* end) LLVMT(LLVMOBF_INLINE_FP());
	LLVM_NOOPT extern bool screen_make_bubble_for_combat_ent(Entity& ent, int* obegin, int* oend, float scale) LLVMT(LLVMOBF_INLINE_FP());
}
