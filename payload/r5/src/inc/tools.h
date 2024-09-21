#pragma once

namespace tools
{
    void replace_slash_with_underscore(const char *path, char *output);
    char integer2char(int digit);
    void *vft_cave_setup(size_t table_count, uptr cave_begin, u32 cave_cap, u32 &disp);
    size_t vft_calc_len(void *ptbl);
}
