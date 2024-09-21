#pragma once

namespace nv::entity
{
	NETVAR_OFFSET_RECEIVER(life_state);
	NETVAR_OFFSET_RECEIVER(team_num);
	NETVAR_OFFSET_RECEIVER(team_member_index);
	NETVAR_OFFSET_RECEIVER(squad_id);
	NETVAR_OFFSET_RECEIVER(trav_progress);
	NETVAR_OFFSET_RECEIVER(trav_start_time);
	NETVAR_OFFSET_RECEIVER(trav_release_time);
	NETVAR_OFFSET_RECEIVER(identifier_base);
	NETVAR_OFFSET_RECEIVER(health);
	NETVAR_OFFSET_RECEIVER(health_max);
	NETVAR_OFFSET_RECEIVER(armor);
	NETVAR_OFFSET_RECEIVER(armor_max);
	NETVAR_OFFSET_RECEIVER(fflags);
	NETVAR_OFFSET_RECEIVER(bleed_out_state);
	NETVAR_OFFSET_RECEIVER(latest_primary_weapon);
	NETVAR_OFFSET_RECEIVER(view_model);
	NETVAR_OFFSET_RECEIVER(pcollision);

	extern void init();
}

class Entity
{
public:
	Entity() = delete;
	Entity(u64 base)
	{
		m_base = base;
	}

	auto base() const { return m_base; }

	template <typename T>
	T &get(u32 off) { return *(T *)(m_base + off); }

	DATA_FIELD_NR(renderable, m_base, void *, 0x10);
	DATA_FIELD_NR(networkable, m_base, void *, 0x18);
	DATA_FIELD_NR(collision, m_base, Collision, nv::entity::pcollision);

	DATA_FIELD(handle, ehandle, 0x8);
	DATA_FIELD(mdl_name, char *, 0x30);	  // m_ModelName
	DATA_FIELD(life_state, short, nv::entity::life_state); // m_lifeState
	DATA_FIELD(time_base, float, STATIC_OFFSET(0x2088)); // m_currentFramePlayer.timeBase
	DATA_FIELD(bone_matrix, matrix3x4a *, 0x0E18);

	DATA_FIELD(name_index, int, 0x38);

	DATA_FIELD(team_num, int, nv::entity::team_num);
	DATA_FIELD(team_member_index, int, nv::entity::team_member_index);
	DATA_FIELD(squad_id, int, nv::entity::squad_id);
	DATA_FIELD(trav_progress, float, nv::entity::trav_progress);	  // m_traversalProgress
	DATA_FIELD(trav_start_time, float, nv::entity::trav_start_time);	  // m_traversalStartTime
	DATA_FIELD(trav_release_time, float, nv::entity::trav_release_time); // m_traversalReleaseTime

	DATA_FIELD(identifier_base, u64, nv::entity::identifier_base); // m_iSignifierName

	DATA_FIELD(health, int, nv::entity::health);		// m_iHealth
	DATA_FIELD(health_max, int, nv::entity::health_max); // m_iMaxHealth
	DATA_FIELD(armor, int, nv::entity::armor);		// m_shieldHealth
	DATA_FIELD(armor_max, int, nv::entity::armor_max);	// m_shieldHealthMax

	DATA_FIELD(flags, int, nv::entity::fflags); // m_fFlags

	DATA_FIELD(bleed_out_state, int, nv::entity::bleed_out_state); // m_bleedoutState

	DATA_FIELD(latest_primary_weapon, ehandle, nv::entity::latest_primary_weapon); // m_latestPrimaryWeapons
	DATA_FIELD(view_model, ehandle, nv::entity::view_model);			// m_hViewModels

	DATA_FIELD(origin, Vec3, 0x17C);
	DATA_FIELD(punch_angle, Vec3, STATIC_OFFSET(0x2438));						   // m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
	DATA_FIELD(last_vis_time, float, STATIC_OFFSET(0x1990));
	DATA_FIELD(view_angle, Vec3, gctx->offsets.player.view_angle); // CInput::CreateMove
	DATA_FIELD(view_pos, Vec3, gctx->offsets.player.view_offset);
	DATA_FIELD(rgfl_coordinate_frame, matrix3x4, STATIC_OFFSET(0x7B0)); //  CClientRenderable::GetRgflCoordinateFrame (40 53 48 83 EC 20 48 8D 59 F0 48 8B CB E8 ? ? ? ? 48 8D 83 ? ? ? ? 48 83 C4 20 5B C3)

	inline bool is_alive()
	{
		if (life_state() != 0)
			return false;
		if (!is_dummie() && bleed_out_state() != 0)
			return false;
		return true;
	}

	inline u64 identifier_hash()
	{
		auto id_base = identifier_base();
		if (!id_base)
			return 0;

		return *(u64 *)id_base;
	}

	inline auto eidx()
	{
		return RS_EHANDLE_TO_INDEX(handle());
	}

	inline bool is_valid()
	{
		if (!base())
			return false;
		if (!identifier_base())
			return false;

		return true;
	}

	inline bool is_player()
	{
		return (identifier_hash() == decrypt_constant(encrypt_constant(0x726579616c70)));
	}

	inline bool is_dummie()
	{
		return (identifier_hash() == decrypt_constant(encrypt_constant(0x6d6d75645f63706e)));
	}

	inline bool is_weapon()
	{
		return (identifier_hash() == decrypt_constant(encrypt_constant(0x786E6F70616577)));
	}

	inline bool is_sky_diving() // #STR: "Player_IsSkyDiving"
	{
		return *reinterpret_cast<u32*>(base() + STATIC_OFFSET(0x468C)) != 0;
	}

	inline auto get_level() // #STR: "XP %d too low"
	{
		auto opaque1 = *reinterpret_cast<int*>(gctx->game_base + STATIC_OFFSET(0x74916C4));
		auto opaque2 = reinterpret_cast<int*>(gctx->game_base + STATIC_OFFSET(0x74917C0));

		if(opaque1 < 1)
			return 1;

		auto num_xp = *reinterpret_cast<int*>(base() + STATIC_OFFSET(0x3694));
		if(num_xp > 0)
		{
			auto v3 = opaque1;
  			auto v4 = opaque1;

			while (num_xp < opaque2[v4])
			{
				--v3;
				if (--v4 < 1)
					return 1;
			}
			return v3;
		}	
		return 0;	
	}

	u64 get_active_weapon();
	u64 get_view_model();
	bool is_ally_of(Entity &other);

	auto get_bone_pos(int id)
	{
		auto bm = bone_matrix();

		if (bm == nullptr || !MmIsAddressValid(bm))
			return VEC_EMPTY;

		return Vec3(
				   bm[id][0][3],
				   bm[id][1][3],
				   bm[id][2][3]) +
			   origin();
	}

	auto get_center_pos()
	{
		auto result = VEC_EMPTY;
		auto pcoll = collision();
		if(pcoll)
		{
			auto vec_mins = pcoll->vec_mins();
			auto vec_maxs = pcoll->vec_maxs();

			_fp(diver, 2.f);

			result = origin();
			result.z += (vec_mins.z + vec_maxs.z) / diver;
		}
		return result;
	}
protected:
	u64 m_base;
};
