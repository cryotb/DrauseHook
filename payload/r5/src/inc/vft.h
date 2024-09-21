#pragma once

namespace vft
{
    LLVM_NOOPT void *cave_setup(size_t table_count, uptr cave_begin, u32 cave_cap, u32 &disp);
    LLVM_NOOPT size_t calc_len(void *ptbl);
}

