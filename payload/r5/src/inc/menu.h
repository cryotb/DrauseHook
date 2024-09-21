#pragma once

namespace menu
{
	struct state_t
	{
		bool inited;
		bool visible;
		int idx;
		int height;
		int nitems;
		int pos_x;
		int pos_y;
		int pos_w;
		int pos_h;

		int render_pos_x;
		int render_pos_y;
	} extern state;

	inline bool is_visible() { return state.visible; }

    void checkbox(respawn::Mat_system_surface *mss, int &idx, const char *text, bool &pval);
    void slider_percentage(respawn::Mat_system_surface *mss, int &idx, const char *text, int &pval, int step);
    void init();
    void on_render(respawn::Mat_system_surface *mss);
}
