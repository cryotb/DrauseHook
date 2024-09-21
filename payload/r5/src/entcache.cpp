#include "inc/include.h"

namespace EC
{
    record* _table = nullptr;
    size_t _table_len = 0;
    size_t _table_cap = 0;
    session_data _session_data;
}

/*
 * Initial setup for the entity cache.
  * allocates required lists and sets boundaries.
*/
bool EC::init()
{
    _table_cap = RS_ENTITY_LIST_SPACE;
    _table_len = (_table_cap * sizeof(record));

    if(!_table_cap || !_table_len)
        return false;

    _table = reinterpret_cast<record*>( g.ix.mm->alloc(_table_len) );

    if(_table == nullptr)
        return false;

    reset( );

    return true;
}

/*
 * Performs a reset of entity cache.
  * no data will be preserved after a call to here.
*/
void EC::reset()
{
    memset( _table, 0, _table_len );
    memset( &_session_data, 0, sizeof(_session_data) );
}

void EC::tick(Entity& me)
{
    if(detail::should_refresh())
    {
        _session_data.tick_last_refreshed = Plat_FloatTick();
        update(me);
    }
}

void EC::render(respawn::Mat_system_surface* mss)
{
    show_debug_stats(mss);
}

void EC::show_debug_stats(respawn::Mat_system_surface* mss)
{
#if defined(DECLARE_AS_DEBUG_BUILD)
    int push = 0;

    int start_x = (g.screen_dim[0] / 2) - 400, start_y = 25;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "EC_Tbl: %p", _table);
    push += 15;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "EC_Cap: %i", _table_cap);
    push += 15;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "EC_tick_last_refreshed: %i", _session_data.tick_last_refreshed);
    push += 15;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "EC_num_invalidations: %i", _session_data.stats.num_invalidations);
    push += 15;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "EC_HighestEIDX: %i", get_highest_eidx());
    push += 15;

    mss->draw_text(2, 14, {start_x, start_y + push}, rs::clr::white, "R5_MaxClients: %i", g.gvars->MaxClients);
    push += 15;
#endif
}

void EC::update(Entity& me)
{
    int highest_eidx = get_highest_eidx();

    auto my_origin = me.origin();

    for (int i = 0; i <= highest_eidx; i++)
    {
        auto field = &g.ent_list[i];
        if (!field->ptr)
            continue;

        auto rec = get(i);
        auto ent_base = field->ptr;

        auto ent = Entity(ent_base);

        if(!handle_generic(ent, rec))
        {
            invalidate(rec);
            continue;
        }

        /* STARTING FROM HERE, WE GOT ENTITY SPECIFIC LIVE DATA (OR NOT) */

        if (ent.is_player() || ent.is_dummie())
        {
            handle_combat_entity(me, my_origin, ent, rec);
        }
    }
}

bool EC::handle_generic(Entity &ent, record *rec)
{
    /* we only want valid entities. */
    if (!ent.is_valid())
        return false;

    /* at this point we only know it's a valid entity.
     * let's do some core details caching so we don't need to
     * request these expensive infos again about the current thing.
     */
    bool has_cache_avail = (rec->valid && rec->eidx != -1);
    if(!has_cache_avail)
    {
        /* first occurrence */
        if(!make_initial_record_cache(ent, rec))
        {
            /* should not happen, but if it did, it seems like we couldn't
             * cache given entity at this time. that's weird! */
            return false;
        }
    } else
    {
        if(!is_cached_record_valid(ent, rec))
        {
            /* seems like our cached record has expired :( */
            return false;
        } 

        /* if it's still good, we got nothing to do, yay :) */
    }

    return true;
}

void EC::handle_combat_entity(Entity& me, const Vec3& my_origin, Entity& ent, record* rec) LLVMT( LLVMOBF_INLINE_FP() )
{
    /* we got an entity of our interest, that being related to combat. */

    rec->is_alive = ent.is_alive();

    if(rec->is_alive_prev == true && rec->is_alive == false)
    {
        /* seems like entity has just died. */
        rec->bounds[0] = 0.f;
        rec->bounds[1] = 0.f;
        rec->bscr_head_avail = false;
        rec->bscr_origin_avail = false;
        rec->bscr_center_avail = false;

        rec->vec_head[0] = VEC_EMPTY;
        rec->vec_head[1] = VEC_EMPTY;
        rec->vec_origin[0] = VEC_EMPTY;
        rec->vec_origin[1] = VEC_EMPTY;
        rec->vec_center[0] = VEC_EMPTY;
        rec->vec_center[1] = VEC_EMPTY;
    }

    rec->is_alive_prev = rec->is_alive;

    if (rec->is_alive)
    {
        bool ignore_range = ( g.conf.visual_far == true || me.is_sky_diving() );

        // skip objects that are too far.
        // if DIST exceeds LIMIT AND should NOT IGNORE_RANGE : mark it not available ;
        // if FAR_ESP is TRUE or LOCAL_PLAYER_IS_SKY_DIVING THEN IGNORE_RANGE TRUE ;
        bool outside_range = rec->dist_to_local > 10000.f && !ignore_range;

        rec->vec_origin[0] = ent.origin();
        rec->vec_head[0] = ent.get_bone_pos(12);
        rec->vec_center[0] = ent.get_center_pos();

        if (!outside_range)
        {
            rec->bscr_origin_avail = respawn::w2s(rec->vec_origin[0], rec->vec_origin[1]);
            rec->bscr_head_avail = respawn::w2s(rec->vec_head[0], rec->vec_head[1]);
            rec->bscr_center_avail = respawn::w2s(rec->vec_center[0], rec->vec_center[1]);

            if (!rec->bscr_head_avail || !rec->bscr_origin_avail)
            {
                rec->fov_to_local = 2000.f;
            }
            else
            {
                rec->fov_to_local = math::get_fov_difference(me.view_angle(), me.view_pos(), rec->vec_center[0]);
            }
        } else
        {
            rec->bscr_origin_avail = false;
            rec->bscr_head_avail = false;
            rec->bscr_center_avail = false;

            rec->fov_to_local = 2000.f;
        }

        rec->is_visible = ent.last_vis_time() >= Plat_FloatTime();

        memcpy(&rec->rgfl_coordinate_frame, &ent.rgfl_coordinate_frame(), sizeof(matrix3x4));
        auto pcoll = ent.collision();
        if (pcoll != nullptr)
        {
            rec->vec_bounds[0] = pcoll->vec_mins();
            rec->vec_bounds[1] = pcoll->vec_maxs();
        }
        rec->rpts_avail = respawn::setup_render_points(
            rec->rgfl_coordinate_frame,
            rec->vec_bounds[0],
            rec->vec_bounds[1],
            rec->rpts);

        if (!rec->tick_last_live_update || rec->tick_last_live_update != Plat_FloatTick())
        {
            rec->is_ally_of_local = ent.is_ally_of(me);
            rec->health[0] = ent.health();
            rec->health[1] = ent.health_max();
            rec->armor[0] = ent.armor();
            rec->armor[1] = ent.armor_max();

            if (!outside_range)
            {
                if (rec->bscr_head_avail && rec->bscr_origin_avail)
                {
                    rec->bounds[1] = math::fabsf(rec->vec_head[1].y - rec->vec_origin[1].y);
                    float ratio = 2.5f;
                    rec->bounds[0] = (rec->bounds[1] / ratio);
                } else
                {
                    rec->bounds[0] = 0.f;
                    rec->bounds[1] = 0.f;
                }
            } else
            {
                rec->bounds[0] = 0.f;
                rec->bounds[1] = 0.f;
            }

            rec->tick_last_live_update = Plat_FloatTick();
        }

        /*
         * Below goes data fetches that are not time critical. 
        */
        if (!rec->time_last_deferred_update || (Plat_FloatTime() - rec->time_last_deferred_update) > 1.f)
        {
            /*
             * Determine the entity color theme.
             */
            rec->clr = LColor(252, 131, 56);
            if (rec->is_ally_of_local)
                rec->clr = LColor(0, 206, 252);
            rec->dist_to_local = math::sqrtf(rec->vec_origin[0].dist_to(my_origin));

            rec->time_last_deferred_update = Plat_FloatTime();
        }

        if(!rec->is_ally_of_local)
        {
            if(rec->is_visible)
            {
                rec->clr = LColor(255, 65, 65);
            }
        }
    }
}

void EC::invalidate(record* rec)
{
    memset(rec, 0, sizeof(record));

    rec->eidx = -1;
    rec->ptr = 0;
    rec->identifier_hash = 0;
    rec->valid = false;

    ++_session_data.stats.num_invalidations;
}

EC::record* EC::get(int idx)
{
    if(idx >= _table_cap)
        return nullptr;

    return &_table[ idx ];
}

int EC::get_highest_eidx()
{
    int result = g.gvars->MaxClients;

    if(core::is_firing_range())
        result = _table_cap;

     if(result >= _table_cap)
        result = _table_cap-1;

    return result;
}

bool EC::is_cached_record_valid(Entity& ent, record* rec)
{
    if(!ent.is_valid())
        return false;

    if(ent.eidx() == -1)
        return false;

    if(ent.eidx() != rec->eidx)
        return false;

    if(ent.base() != rec->ptr)
        return false;

    return true;
}

bool EC::make_initial_record_cache(Entity& ent, record* rec)
{
    rec->eidx = ent.eidx();
    rec->ptr = ent.base();
    rec->identifier_hash = ent.identifier_hash();
    rec->valid = true;

    if (ent.is_dummie())
    {
        strcpy(rec->name, _XS("dummy"));
    }
    else if(ent.is_player())
    {
        rec->level = ent.get_level();
        if (!respawn::get_name(ent, rec->name, sizeof(rec->name)))
            strcpy(rec->name, _XS("????"));
    }

    return true;
}

bool EC::detail::should_refresh()
{
    return true;
    if(!is_valid_session())
        return false;

    /* updating cached entity core/base data. */

    if(!_session_data.tick_last_refreshed)
        return true;

    int tick_delta = Plat_FloatTick() - _session_data.tick_last_refreshed;
    /* for now, check cached core/base data every tick change. */
    return (tick_delta > 0);
}
