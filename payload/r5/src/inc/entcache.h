#pragma once

namespace EC
{
    /*
     * A snapshot of an entity's version.
     *
     * WARNING: MAKE SURE TO USE ENTITY INSTANCE'S OWN VALIDATION FEATURES BEFORE USING THIS DATA!
     * IT MIGHT BE NOT IN SYNC WITH WHAT THE CURRENT EXECUTING THREAD IS IN STATE OF, BEWARE!
     * THIS IS MOSTLY FOR INTERNAL USAGE
     * DO NOT IGNORE THIS, OR YOU MAY CORRUPT DATA AND CRASH!
     *  
    */
    struct record
    {
        int eidx;
        uptr ptr;
        bool valid;

        u64 identifier_hash;

        char name[RS_PLAYER_MAX_NAME_LEN];

        int tick_last_live_update;
        float time_last_deferred_update;
        float last_vis_time;

        bool is_alive;
        bool is_alive_prev;
        bool is_ally_of_local;
        bool is_visible;

        int health[2];
        int armor[2];
        int level;

        Vec3 vec_origin[2];
        Vec3 vec_head[2];
        Vec3 vec_bounds[2];
        Vec3 vec_center[2];

        matrix3x4 rgfl_coordinate_frame;
        rs::math::render_points_t rpts;
        
        bool rpts_avail;
        bool bscr_origin_avail;
        bool bscr_head_avail;
        bool bscr_center_avail;

        float dist_to_local;
        float fov_to_local;
        float bounds[2];

        LColor clr;
    };

    /*
     * Contains data that is only valid throughout the current game
      * or as said above, current session. reset this every level shutdown!
    */
    struct session_data
    {
        int tick_last_refreshed;

        struct
        {
            int num_invalidations;
        } stats;
    };

    extern record* _table;
    extern size_t _table_len;
    extern size_t _table_cap;
    extern session_data _session_data;

    bool init();
    void reset();
    void tick(Entity& me);
    void render(respawn::Mat_system_surface* mss);
    void show_debug_stats(respawn::Mat_system_surface* mss);
    void update(Entity& me);
    bool handle_generic(Entity& ent, record* rec);
    LLVM_NOOPT void handle_combat_entity(Entity& me, const Vec3& my_origin, Entity& ent, record* rec);
    void invalidate(record* rec);
    record* get(int idx);

    int get_highest_eidx();

    bool is_cached_record_valid(Entity& ent, record* rec);
    bool make_initial_record_cache(Entity& ent, record* rec);

    /*
     * EntityCache -> DETAIL / INTERNAL section.
    */
    namespace detail
    {
        bool should_refresh();
    }
}
