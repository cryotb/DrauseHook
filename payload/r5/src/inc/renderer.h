#pragma once

namespace render
{
    inline respawn::Mat_system_surface* mss = nullptr;

    void line(const Point2& start, const Point2& end, const LColor& clr);
    void rect(const Point2& start, const Point2& end, const LColor& clr);
    void rect_filled(const Point2& start, const Point2& end, const LColor& clr);
}
