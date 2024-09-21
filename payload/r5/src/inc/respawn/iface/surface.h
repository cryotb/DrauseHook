#pragma once

namespace respawn
{
    class Mat_system_surface
    {
    public:
        void get_screen_size(int &wide, int &tall)
        {
            return vfunc::call<53, void>(this, &wide, &tall);
        }

        void draw_set_color(const LColor &clr)
        {
            return vfunc::call<14, void>(this, clr.r, clr.g, clr.b, clr.a);
        }

        void draw_rect_outlined(int x0, int y0, int x1, int y1)
        {
            return vfunc::call<19, void>(this, x0, y0, x1, y1);
        }

        void draw_rect_filled(int x0, int y0, int x1, int y1)
        {
            auto addr = gctx->game_base + gctx->offsets.fns.mss_draw_filled_rect;
            return reinterpret_cast<void(__thiscall*)(void*, int, int, int, int)>(PTR_OF(addr))(
                this, x0, y0, x1, y1);
        }

        void draw_line(int x0, int y0, int x1, int y1)
        {
            return vfunc::call<20, void>(this, x0, y0, x1, y1);
        }

        template <typename... A>
        void draw_text(u16 font, int size, const Point2& pos, const LColor &clr, const char *fmt, A... args)
        {
            using fn = void(__thiscall *)(void *, u16, int, int, int, int, int, int, int, const char *, ...);
            auto fun = reinterpret_cast<fn>((*reinterpret_cast<uptr **>(this))[195]);

            return fun(this, font, size, pos.x, pos.y,
                                    clr.r, clr.g, clr.b, clr.a, fmt, args...);
        }
    };
}
