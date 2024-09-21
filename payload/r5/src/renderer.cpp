#include "inc/include.h"

namespace render
{
    void line(const Point2& start, const Point2& end, const LColor& clr)
    {
        mss->draw_set_color(clr);
        mss->draw_line(start.x, start.y, end.x, end.y);
    }

    void rect(const Point2& start, const Point2& end, const LColor& clr)
    {
        mss->draw_set_color(clr);
        mss->draw_rect_outlined(start.x, start.y, end.x, end.y);
    }

    void rect_filled(const Point2& start, const Point2& end, const LColor& clr)
    {
        mss->draw_set_color(clr);
        mss->draw_rect_filled(start.x, start.y, end.x, end.y);
    }
}
