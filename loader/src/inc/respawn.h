#pragma once

namespace respawn
{
    bool collect_offsets(Context* ctx, LoaderContext* lctx, uptr game_image_base, uptr game_cs_base, uptr game_cs_len, bool from_snap = false);
}
