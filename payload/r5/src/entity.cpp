#include "inc/include.h"

void nv::entity::init()
{
	NETVAR_ASSIGN(&life_state, "DT_Player", "m_lifeState");
	NETVAR_ASSIGN(&team_num, "DT_BaseEntity", "m_iTeamNum");
	NETVAR_ASSIGN(&team_member_index, "DT_BaseEntity", "m_teamMemberIndex");
	NETVAR_ASSIGN(&squad_id, "DT_BaseEntity", "m_squadID");
	NETVAR_ASSIGN(&trav_progress, "DT_LocalPlayerExclusive", "m_traversalProgress");
	NETVAR_ASSIGN(&trav_start_time, "DT_LocalPlayerExclusive", "m_traversalStartTime");
	NETVAR_ASSIGN(&trav_release_time, "DT_LocalPlayerExclusive", "m_traversalReleaseTime");
	NETVAR_ASSIGN(&identifier_base, "DT_BaseEntity", "m_iSignifierName");
	NETVAR_ASSIGN(&health, "DT_Player", "m_iHealth");
	NETVAR_ASSIGN(&health_max, "DT_Player", "m_iMaxHealth");
	NETVAR_ASSIGN(&armor, "DT_BaseEntity", "m_shieldHealth");
	NETVAR_ASSIGN(&armor_max, "DT_BaseEntity", "m_shieldHealthMax");
	NETVAR_ASSIGN(&fflags, "DT_Player", "m_fFlags");
	NETVAR_ASSIGN(&bleed_out_state, "DT_Player", "m_bleedoutState");
	NETVAR_ASSIGN(&latest_primary_weapon, "DT_BaseCombatCharacter", "m_latestPrimaryWeapons");
	NETVAR_ASSIGN(&view_model, "DT_Player", "m_hViewModels");	
	NETVAR_ASSIGN(&pcollision, "DT_BaseEntity", "m_Collision");
}

u64 Entity::get_active_weapon()
{
	int index = 0;
	ehandle handle = latest_primary_weapon();
	if (!handle)
		return 0;

	index = RS_EHANDLE_TO_INDEX(handle);
	if (index == -1)
		return 0;
	if (index >= RS_ENTITY_LIST_SPACE)
		return 0;

	return g.ent_list[index].ptr;
}

u64 Entity::get_view_model()
{
	int index = 0;
	ehandle handle = view_model();
	if (!handle)
		return 0;

	index = RS_EHANDLE_TO_INDEX(handle);
	if (index == -1)
		return 0;
	if (index >= RS_ENTITY_LIST_SPACE)
		return 0;

	return g.ent_list[index].ptr;
}

bool Entity::is_ally_of(Entity &other)
{
	if (is_dummie() || other.is_dummie())
		return false;

	auto game_mode = g.game_mode;

	switch (game_mode)
	{
	case GameModes::survival:
		return (other.team_num() == team_num());
	case GameModes::control:
	case GameModes::tdm:
		return (RS_CONVERT_TEAM_MODULO(other.team_num()) == RS_CONVERT_TEAM_MODULO(team_num()));
	case GameModes::gunrun:
		return (other.squad_id() == squad_id());
	}

	/*
	 *	Default case should never happen, unless game has updated and a new mode was added.
	 *	Make sure to account for this later at some point, as it could be dangerous.
	 */

	return false;
}
