// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "ConnectionTicket.h"
#include "Projectile.h"
#include "core/StringName.h"
#include "lua/LuaWrappable.h"
#include "scenegraph/Model.h"
#include <bitset>
#include <string>

class ModelBody;

/**
 * Class: GunManager
 *
 * The GunManager is responsible for managing all combat-related aspects of ship
 * weaponry.
 *
 * The typical lifecycle of the GunManager component is as follows:
 * - A GunManager is constructed when a ship is created or its type changed
 * - It is informed about the available weapon mounts on a ship shortly after construction
 * - Weapons are attached to those mounts by the equipment code
 * - Weapons are assigned group indices to allow separate fire control and targeting
 * - Targets are assigned to individual groups
 * - A higher-level input manager sends fire commands for each group
 * - Weapon mounts are added or removed as needed by equipment attached to the ship
 * - Finally, when the ship is destroyed or its type changed, the GunManager is destroyed
 */
class GunManager : public LuaWrappable {
public:
	static constexpr size_t NUM_WEAPON_INDICES = 256;
	using WeaponIndexSet = std::bitset<NUM_WEAPON_INDICES>;

	struct WeaponMount;

	enum ProjectileType : uint8_t { // <enum scope='GunManager' name='ProjectileType' prefix='PROJECTILE_' public>
		PROJECTILE_BALLISTIC, // Impact weapons: pulsecannons, railguns, etc.
		PROJECTILE_BEAM,      // Instantaneous-hit weapons: lasers
	};

	struct ProjectileDef {
		float lifespan = 1;
		float speed = 1;

		// Damage
		float impactDamage = 0.0; // How much damage is dealt when this projectile hits a target
		float explosiveDamage = 0.0; // When this projectile detonates (impact or proximity fuse) how much damage is dealt in the radius?
		float explosionRadius = 0.0; // How large of an explosion radius does this projectile have?
		float explosionFalloff = 1.0; // How does the damage fall off from the center to the edge of the explosion? 2.0 = quadratic, 1.0 = linear, 0.0 = no falloff, use "impact area" approximation instead
		// TODO: damage type

		// Feature flags
		uint8_t isMining : 1; // Does this projectile deal "mining" damage? TODO: better damage type system
		uint8_t hasProxyFuse : 1; // Does this projectile have a proximity fuse?
		uint8_t proxyIFF : 1; // Is the proximity fuse capable of reading IFF data?

		// Fusing
		float proxyFuseRadius = 0.0; // This projectile will detonate when it detects a body within this range
		float proxyFuseArmTime = 0.0; // How long after firing before the projectile fuse is armed?

		// Visual settings
		float length = 0;
		float width = 0;
		Color color;
	};

	// Information about a specific "weapon type"
	// TODO: create one of these in Lua per weapon definition and reference them from each mounted gun
	// TODO: create a separate projectile definition and reference it
	struct WeaponData {
		float firingRPM = 1;	// number of shots per minute (60s / time between shots)
		float firingHeat = 0; // amount of thermal energy(kJ) emitted into the system per shot

		//TODO: integrate this with a whole-ship cooling system
		float coolingPerSecond = 0; // nominal amount of thermal energy removed per second (kW)
		float overheatThreshold = 1; // total amount of thermal energy(kJ) the gun can store while functioning

		ProjectileType projectileType = PROJECTILE_BALLISTIC;
		uint8_t numBarrels = 1;		 // total number of barrels on the model
		bool staggerBarrels = false; // should we fire one barrel after another, or both at the same time?

		ProjectileData projectile; // deprecated, to replace with RefCountedPtr<ProjectileDef>
		std::string modelPath; // model to render this weapon with
	};

	// Information about a specific mounted weapon (serialized directly)
	struct WeaponState {
		WeaponMount *mount;					// Mounted hardpoint
		uint8_t group = 0;					// Group this hardpoint belongs to
		bool withinGimbalLimit = false;		// does the gun have a valid firing solution on the target?

		uint8_t lastBarrel = 0;	 // the last barrel used to fire (for multi-barrel weapons)
		float temperature = 0;	 // current gun temperature
		double nextFireTime = 0; // time at which the gun will be ready to fire again
		// TODO: integrate this with a whole-ship cooling system
		// Currently uses GunManager::m_coolingBoost
		// float coolerOverclock = 1.0; // any boost to gun coolers from ship equipment
		// float powerOverclock = 1.0; // boost to overall firerate (and corresponding increase in heat generation)

		vector3f currentLead;
		vector3d currentLeadPos;

		WeaponData data;
		SceneGraph::Model *model; // gun model, currently unused
	};

	// Information about a specific mount that a weapon is attached to
	// Currently only handles gimballed weapon mounts, but may support turrets in the future
	struct WeaponMount {
		StringName id;
		SceneGraph::Tag *tag;     // Tag in the parent model that this weapon mount is attached to
		vector2f gimbalLimitTan;  // tangent of gimbal limits
		//SceneGraph::Model *model; // model to render for this weapon mount, if any
		// TODO: enable/disable hardpoint based on ship configuration, i.e. landing/vtol/wings?
	};

	// Combines one or more weapons with a shared fire-control trigger and targeting information
	struct GroupState {
		WeaponIndexSet weapons;				// Whic weapons are assigned to this group?
		const Body *target;					// The target for this group, if any
		uint32_t target_idx;				// Group target body index for serialization, used in PostLoadFixup()
		bool firing;						// Is the group currently firing
		bool fireWithoutTargeting;			// Can the group fire without a target/target not in gimbal range?
		ConnectionTicket onTargetDestroyed;	// Unlock the target once it's been destroyed
	};

	GunManager() = default;

	void Init(ModelBody *b);

	void SaveToJson(Json &jsonObj, Space *space);
	void LoadFromJson(const Json &jsonObj, Space *space);

	void PostLoadFixup(Space *space);

	void StaticUpdate(float deltaTime);

	// ==========================================

	// Add a weapon mount to this gun manager.
	// Returns false if a hardpoint already exists on this GunManager with the specified name.
	bool AddWeaponMount(const StringName &id, const StringName &tagName, vector2f gimbalLimitDegrees);
	// Remove a weapon mount from this gun manager.
	// The caller should always ensure that the weapon mount is empty before calling this function.
	// Returns false if the mount does not exist or is not empty.
	bool RemoveWeaponMount(const StringName &id);

	// Attach a weapon to a specific mount.
	// Returns false if the hardpoint cannot be found or the weapon could not be mounted.
	bool MountWeapon(const StringName &hardpoint, const WeaponData &data);
	// Remove the attached weapon from a specific mount
	void UnmountWeapon(const StringName &hardpoint);
	// Check if any weapon is attached to a specific mount
	bool IsWeaponMounted(const StringName &hardpoint) const;

	const std::vector<WeaponState> &GetWeapons() const { return m_weapons; }

	uint32_t GetNumWeapons() const { return m_weapons.size(); }
	const WeaponState *GetWeaponState(uint32_t numWeapon) const { return m_weapons.size() > numWeapon ? &m_weapons[numWeapon] : nullptr; }
	uint32_t GetWeaponIndexForHardpoint(const StringName &hardpoint) const;

	// ==========================================

	// For AI/NPC ship usage, combines all weapons into a single group
	void SetupDefaultGroups();
	void SetTrackingTarget(const Body *target) { return SetGroupTarget(0, target); }
	void SetAllGroupsFiring(bool firing = false);

	// For player weapon management

	// Assign a specific weapon to a firing group
	// Weapons are by default all assigned to group 0
	void AssignWeaponToGroup(uint32_t numWeapon, uint32_t group);
	// Remove a weapon group in its entirety
	void RemoveGroup(uint32_t group);

	const std::vector<GroupState> &GetWeaponGroups() const { return m_groups; }

	const Body *GetGroupTarget(uint32_t group);
	void SetGroupTarget(uint32_t group, const Body *target);
	void SetGroupFiring(uint32_t group, bool firing);
	// Sets whether the group will authorize individual weapons to fire without a target within the gimbal arc of the weapon
	void SetGroupFireWithoutTargeting(uint32_t group, bool enabled);

	// TEMP: return the average lead position of a firing group - should query individual weapons instead
	vector3d GetGroupLeadPos(uint32_t group);
	// TEMP: return the average temperature state of a weapon group in 0..1
	float GetGroupTemperatureState(uint32_t group);

	// ==========================================

	bool IsFiring() const { return m_isAnyFiring; }
	bool IsGroupFiring(uint32_t group) const { return m_groups.size() > group ? m_groups[group].firing : false; }

	// TODO: separate this functionality to a ship-wide cooling system
	void SetCoolingBoost(float boost) { m_coolingBoost = boost; }

	// Semi-hacky, would like to replace with a better eventing system of some sort
	WeaponIndexSet GetGunsFiredThisFrame() const { return m_firedThisFrame; }
	WeaponIndexSet GetGunsStoppedThisFrame() const { return m_stoppedThisFrame; }

private:

	// Handle checking and firing a given gun.
	// Note that this currently does not nicely handle spawning multiple projectiles per timestep - i.e. timewarp or a weapon RPM higher than 3600
	// Projectile spawns are also "snapped" to the start of a timestep if they are not direct multiples of the timestep duration
	void Fire(WeaponState &weapon);

	// Calculate the position a given gun should aim at to hit the current target body
	// This is effectively the position of the target at T+n
	void CalcWeaponLead(WeaponState &state, vector3d position, vector3d relativeVelocity, vector3d relativeAccel);

	const matrix4x4f &GetMountTransform(WeaponState &weapon);

	void RemoveGroupIndex(WeaponIndexSet &group, uint32_t index);

	std::vector<GroupState> m_groups;
	std::vector<WeaponState> m_weapons;
	std::map<StringName, WeaponMount> m_mounts;

	WeaponIndexSet m_firedThisFrame;
	WeaponIndexSet m_stoppedThisFrame;
	WeaponIndexSet m_stoppedNextFrame;

	ModelBody *m_parent = nullptr;
	bool m_isAnyFiring = false;
	float m_coolingBoost = 1.0;
};
