/* 
 * Copyright © 2008-2024 Pioneer Developers. 
 * See AUTHORS.txt for details.
 * 
 * Licensed under the terms of the GPL v3. 
 * See licenses/GPL-3.txt
 */

#pragma once

#include "ConnectionTicket.h"
#include "Projectile.h"
#include "core/StringName.h"
#include "lua/LuaWrappable.h"
#include "scenegraph/Model.h" // Might use forward declaration instead
#include <bitset>
#include <map>
#include <string>
#include <vector>

// Forward Declarations for references in this header
class ModelBody;
class Body;
class Space;
class Json;

namespace SceneGraph {
	class Tag;
	class Model;
}

/**
 * \class GunManager
 * \brief Manages combat-related aspects of ship weaponry, including mounting, 
 *        firing, groups, and cooling/overheating logic.
 *
 * Typical lifecycle:
 * - Constructed when a ship is created or its type changes.
 * - Receives info about weapon mounts (e.g. from a ship model).
 * - Equipment code attaches/detaches weapons on those mounts.
 * - Weapons are assigned to groups for separate control.
 * - A target can be assigned to each group.
 * - Input manager sends fire commands for each group.
 * - Finally, destroyed when the ship is destroyed or type changes.
 */
class GunManager : public LuaWrappable {
public:
	static constexpr size_t NUM_WEAPON_INDICES = 256;
	using WeaponIndexSet = std::bitset<NUM_WEAPON_INDICES>;

	struct WeaponMount;

	/**
	 * \enum ProjectileType
	 * \brief Different projectile categories for ship weapons.
	 */
	enum class ProjectileType : uint8_t {
		BALLISTIC, ///< Impact weapons: pulse cannons, railguns, etc.
		BEAM       ///< Instant-hit weapons: lasers, etc.
	};

	/**
	 * \struct ProjectileDef
	 * \brief Definition of how a projectile behaves: damage, fusing, visuals, etc.
	 */
	struct ProjectileDef {
		float lifespan = 1.0f;
		float speed = 1.0f;

		// Damage
		float impactDamage = 0.0f;
		float explosiveDamage = 0.0f;
		float explosionRadius = 0.0f;
		float explosionFalloff = 1.0f; // 2.0 = quadratic, 1.0 = linear, 0.0 = no falloff

		// Feature flags
		uint8_t isMining : 1;
		uint8_t hasProxyFuse : 1;
		uint8_t proxyIFF : 1;

		// Fusing
		float proxyFuseRadius = 0.0f;
		float proxyFuseArmTime = 0.0f;

		// Visual settings
		float length = 0.0f;
		float width = 0.0f;
		Color color; // from Projectile.h
	};

	/**
	 * \struct WeaponData
	 * \brief Stores stats about a weapon type, such as firing rates, projectile kind, etc.
	 */
	struct WeaponData {
		float firingRPM = 60.0f;    ///< Shots per minute
		float firingHeat = 0.0f;   ///< Thermal energy (kJ) introduced per shot
		float coolingPerSecond = 0.0f; 
		float overheatThreshold = 1.0f;

		ProjectileType projectileType = ProjectileType::BALLISTIC;
		uint8_t numBarrels = 1;
		bool staggerBarrels = false;

		// Deprecated. Possibly replaced by a refcounted pointer to ProjectileDef
		ProjectileData projectile;

		std::string modelPath; ///< Path to the weapon's model for rendering
	};

	/**
	 * \struct WeaponState
	 * \brief Represents a mounted weapon instance on the ship.
	 */
	struct WeaponState {
		WeaponMount* mount = nullptr; // The mount where this weapon is attached
		uint8_t group = 0;
		bool withinGimbalLimit = false;

		uint8_t lastBarrel = 0;
		float temperature = 0.0f;
		double nextFireTime = 0.0;

		vector3f currentLead;
		vector3d currentLeadPos;

		WeaponData data;
		SceneGraph::Model* model = nullptr; 
	};

	/**
	 * \struct WeaponMount
	 * \brief Info about a physical mount for a weapon on the ship (gimbal data, etc.).
	 */
	struct WeaponMount {
		StringName id;
		SceneGraph::Tag* tag = nullptr;
		vector2f gimbalLimitTan = vector2f(0.0f); // tangent of gimbal limit angles
	};

	/**
	 * \struct GroupState
	 * \brief A firing group that can contain one or more weapons plus a single target.
	 */
	struct GroupState {
		WeaponIndexSet weapons;
		const Body* target = nullptr;
		bool firing = false;
		bool fireWithoutTargeting = false;
		ConnectionTicket onTargetDestroyed;
	};

	GunManager();
	virtual ~GunManager() override {} ///< Virtual destructor if needed

	// -- Lifecycle --
	void Init(ModelBody* b);

	void SaveToJson(Json& jsonObj, Space* space);
	void LoadFromJson(const Json& jsonObj, Space* space);

	void StaticUpdate(float deltaTime);

	// -- Mount Management --
	bool AddWeaponMount(const StringName& id, const StringName& tagName, vector2f gimbalLimitDegrees);
	bool RemoveWeaponMount(const StringName& id);

	bool MountWeapon(const StringName& hardpoint, const WeaponData& data);
	void UnmountWeapon(const StringName& hardpoint);
	bool IsWeaponMounted(const StringName& hardpoint) const;

	// Possibly allow changing gimbal limits after creation
	bool SetGimbalLimits(const StringName& hardpoint, vector2f newGimbalDegrees);

	const std::vector<WeaponState>& GetWeapons() const { return m_weapons; }
	uint32_t GetNumWeapons() const { return uint32_t(m_weapons.size()); }
	const WeaponState* GetWeaponState(uint32_t numWeapon) const;
	uint32_t GetWeaponIndexForHardpoint(const StringName& hardpoint) const;

	// -- Groups & Firing --
	void SetupDefaultGroups();
	void SetTrackingTarget(const Body* target) { SetGroupTarget(0, target); }
	void SetAllGroupsFiring(bool firing = false);

	void AssignWeaponToGroup(uint32_t numWeapon, uint32_t group);
	void RemoveGroup(uint32_t group);

	const std::vector<GroupState>& GetWeaponGroups() const { return m_groups; }

	const Body* GetGroupTarget(uint32_t group);
	void SetGroupTarget(uint32_t group, const Body* target);
	void SetGroupFiring(uint32_t group, bool firing);
	void SetGroupFireWithoutTargeting(uint32_t group, bool enabled);

	vector3d GetGroupLeadPos(uint32_t group) const;
	float GetGroupTemperatureState(uint32_t group) const;

	bool IsFiring() const { return m_isAnyFiring; }
	bool IsGroupFiring(uint32_t group) const;

	// TODO: a better event-based cooling approach might replace these:
	void SetCoolingBoost(float boost) { m_coolingBoost = boost; }
	float GetCoolingBoost() const { return m_coolingBoost; }

	// Overclock example: speed up firing rate, but generate more heat
	void SetPowerOverclock(float oc) { m_powerOverclock = oc; }
	float GetPowerOverclock() const { return m_powerOverclock; }

	WeaponIndexSet GetGunsFiredThisFrame() const { return m_firedThisFrame; }
	WeaponIndexSet GetGunsStoppedThisFrame() const { return m_stoppedThisFrame; }

private:
	void Fire(WeaponState& weapon);
	void CalcWeaponLead(WeaponState& state, vector3d position, vector3d relativeVelocity, vector3d relativeAccel);
	const matrix4x4f& GetMountTransform(WeaponState& weapon);
	void RemoveGroupIndex(WeaponIndexSet& group, uint32_t index);

	std::vector<GroupState> m_groups;
	std::vector<WeaponState> m_weapons;
	std::map<StringName, WeaponMount> m_mounts;

	WeaponIndexSet m_firedThisFrame;
	WeaponIndexSet m_stoppedThisFrame;
	WeaponIndexSet m_stoppedNextFrame;

	ModelBody* m_parent = nullptr;
	bool m_isAnyFiring = false;
	float m_coolingBoost = 1.0f;
	float m_powerOverclock = 1.0f; ///< Factor that speeds up firing but increases heat
};

