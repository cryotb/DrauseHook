#pragma once

namespace features
{
	extern Entity rotational_find_target(Entity &me, Vec3 &outpos, float max_dist, float max_fov, float dead_zone, bool& should_slow);

	extern void on_rcs(Entity &me, float &x, float &y);
	extern void on_aim_assist(Entity &me, float &x, float &y);
	extern void on_esp(respawn::Mat_system_surface* mss, Entity &me);
	extern void on_movement(Entity& me);
	extern void on_trigger(Entity& me);
	extern void on_anonymizer();
	extern void anonymizer_reset();
}
