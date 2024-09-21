#pragma once

namespace respawn
{
    class View_render
    {
    public:
        auto get_view_matrix(char index)
        {
            return *(d3d::D3DMATRIX **)(BASE_OF(this) + 8 * index + 0x11A350);
        }
    };
}
