// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "JsonFwd.h"
#include "ProjectileData.h"
#include "vector3.h"
#include "sound/Sound.h"

class Body;
class Space;

namespace SceneGraph {
	class Model;
}

enum GunDir {
	GUN_FRONT,
	GUN_REAR,
	GUNMOUNT_MAX = 2
};

typedef int GunId;
typedef int MountId;

class FixedGuns {
public:
	FixedGuns() = delete;
	FixedGuns(Body *b);
	~FixedGuns();

	void SaveToJson(Json &jsonObj, Space *space);
	void LoadFromJson(const Json &jsonObj, Space *space);

	void ParseModelTags(SceneGraph::Model *m);

	bool MountGun(MountId num, const std::string &name, const std::string &sound, const float recharge, const float heatrate, const float coolrate, const int barrels, const ProjectileData &pd);
	bool UnMountGun(MountId num);

	void SetGunsFiringState(GunDir dir, int s);

	// Return value is 'true' whenever a new projectile is fired
	bool UpdateGuns(float timeStep, Body *shooter);

	int GetMountsSize() const { return int(m_mounts.size()); };
	int GetMountedGunsNum() const { return int(m_guns.size()); }
	int GetFreeMountsSize() const { return int(m_mounts.size() - m_guns.size()); };
	MountId FindFirstEmptyMount() const;
	std::vector<MountId> FindEmptyMounts() const;
	MountId FindMountOfGun(const std::string &name) const;

	void SetActivationStateOfGun(GunId num, bool active);
	bool GetActivationStateOfGun(GunId num) const;

/*
	TODO IMP 0:
	MOVE ParseModelTags in Model!!!!!!

	TODO1:
	bool SwapTwoMountedGuns(int gun_a, int gun_b);

	TODO2:
	in Lua:
	1) handle a simple swap in info's view,
	2) put some time in it and allow it only if in a station
	(maybe allow also if you have a crew?)

	TODO3:
	CycleFireModeForGun(num);

	TODO4:
	CreateGroup(num, );
	CycleFireModeForGunGroup(num);
*/

	GunDir IsFront(GunId num) const;
	bool IsFiring() const;
	bool IsFiring(GunId num) const;
	bool IsBeam(GunId num) const;
	float GetGunTemperature(GunId idx) const;
	const std::string &GetGunName(GunId idx) { return m_guns[idx].gun_data.gun_name; };
	inline float GetGunRange(GunId idx) const { return m_guns[idx].gun_data.projData.speed * m_guns[idx].gun_data.projData.lifespan; };
	inline float GetProjSpeed(GunId idx) const { return m_guns[idx].gun_data.projData.speed; };
	inline void SetCoolingBoost(float cooler) { m_cooler_boost = cooler; };

private:

	bool Fire(GunId num, Body *shooter);

	// Structure holding name, position and direction of a mount (loaded from Model data)
	struct Mount {
		std::string name;
		std::vector<vector3d> locs;
		GunDir dir;
	};

	// Structure holding data of a single (maybe with multiple barrel) 'mounted' gun.
	struct GunData {
		GunData() : // Defaul ctor: not to be used
			recharge(0.0f),
			temp_cool_rate(0.0f),
			temp_heat_rate(0.0f),
			barrels(0),
			projData() {}
		GunData(const std::string &n, const std::string &s, float r, float h, float c, int b, const ProjectileData &pd) : // "Faster" ctor
			gun_name(n),
			sound(s),
			recharge(r),
			temp_cool_rate(c),
			temp_heat_rate(h),
			barrels(b),
			projData(pd) {}
		GunData(const GunData& gd) : //Copy ctor
			gun_name(gd.gun_name),
			sound(gd.sound),
			recharge(gd.recharge),
			temp_cool_rate(gd.temp_cool_rate),
			temp_heat_rate(gd.temp_heat_rate),
			barrels(gd.barrels),
			projData(gd.projData) {}
		std::string gun_name;
		std::string sound;
		float recharge;
		float temp_heat_rate;
		float temp_cool_rate;
		int barrels;
		ProjectileData projData;
	};

	// Structure holding actual status of a gun
	struct GunStatus {
		GunStatus() : // Defaul ctor
			mount_id(-1),
			is_firing(false),
			is_active(false),
			recharge_stat(0.0f),
			temperature_stat(0.0f),
			gun_data() {}
		GunStatus(GunId m_id, const std::string &n, const std::string &s, float r, float h, float c, int b, const ProjectileData &pd) : // "Fast" ctor for creation
			mount_id(m_id),
			is_firing(false),
			is_active(true),
			recharge_stat(r),
			temperature_stat(0.0f),
			gun_data(n, s, r, h, c, b, pd) {}
		GunStatus(const GunStatus& gs) : // Copy ctor
			mount_id(gs.mount_id),
			is_firing(gs.is_firing),
			is_active(gs.is_active),
			recharge_stat(gs.recharge_stat),
			temperature_stat(gs.temperature_stat),
			gun_data(gs.gun_data) {}
		GunId mount_id; // <- store the mount sequential number
					  //    or, if equal to -1, the fact that
					  // this gun is "invalid": it should have
					  // been deleted before... But never say
					  // never: this could be used to signal
					  // a broken or damaged gun
		bool is_firing;
		bool is_active;
		float recharge_stat;
		float temperature_stat;
		Sound::Event sound;
		GunData gun_data;
	};

	// Vector with mounts (data coming from models)
	std::vector<Mount> m_mounts;

	// Vector with mounted guns and their status
	std::vector<GunStatus> m_guns;

	//TODO: mmmh... Should I put cooler PER gun?
	float m_cooler_boost;
};

#endif // FIXEDGUNS_H
