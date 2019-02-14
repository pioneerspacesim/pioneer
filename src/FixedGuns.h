// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "JsonFwd.h"
#include "ProjectileData.h"
#include "vector3.h"

class Body;
class Space;

namespace SceneGraph {
	class Model;
}

enum Guns {
	GUN_FRONT,
	GUN_REAR,
	GUNMOUNT_MAX = 2
};

class FixedGuns {
public:
	FixedGuns() = delete;
	FixedGuns(Body *b);
	~FixedGuns();
	void ParseModelTags(SceneGraph::Model *m);
	/*
	TODO2:
	int GetMountsSize();
	const std::string GetMountName(int i);
	bool CheckIsEmpty(int num);
	int FindFirstEmpty();
	bool SwapTwoMountedGuns(int gun_a, int gun_b);

	TODO0:
	Lua... Provide an interface in Ship,
	then try to find a way to pass directly the
	spitted out part FixedGuns.
	TODO1:
	SetActivationStateOfGun(int num, );
	CycleFireModeForGun(num);

	TODO3:
	CreateGroup(num, );
	....
*/

	void UpdateGuns(float timeStep);
	bool Fire(const int num, Body *shooter);

	bool IsFiring();
	bool IsFiring(const int num);
	bool IsBeam(const int num);
	float GetGunTemperature(int idx) const;
	bool MountGun(const int num, const float recharge, const float heatrate, const float coolrate, const int barrels, const ProjectileData &pd);
	bool UnMountGun(int num);
	inline float GetGunRange(int idx) { return m_guns[idx].gun_data.projData.speed * m_guns[idx].gun_data.projData.lifespan; };
	inline float GetProjSpeed(int idx) { return m_guns[idx].gun_data.projData.speed; };
	inline void SetCoolingBoost(float cooler) { m_cooler_boost = cooler; };
	inline void SetGunFiringState(int idx, int s)
	{
		if (idx < m_guns.size())
			m_guns[idx].is_firing = s;
	};

	int GetMountedGunsNum() const { return int(m_guns.size()); }

	void SaveToJson(Json &jsonObj, Space *space);
	void LoadFromJson(const Json &jsonObj, Space *space);

private:
	// Structure holding name, position and direction of a mount (loaded from Model data)
	struct Mount {
		std::string name;
		std::vector<vector3d> locs;
		vector3d dir;
	};

	// Structure holding data of a single (maybe with multiple barrel) 'mounted' gun.
	struct GunData {
		GunData() : // Defaul ctor
			recharge(0.0f),
			temp_cool_rate(0.0f),
			temp_heat_rate(0.0f),
			barrels(0),
			projData() {}
		GunData(float r, float h, float c, int b, const ProjectileData &pd) : // "Faster" ctor
			recharge(r),
			temp_cool_rate(c),
			temp_heat_rate(h),
			barrels(b),
			projData(pd) {}
		GunData(const GunData& gd) : //Copy ctor
			recharge(gd.recharge),
			temp_cool_rate(gd.temp_cool_rate),
			temp_heat_rate(gd.temp_heat_rate),
			barrels(gd.barrels),
			projData(gd.projData) {}
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
			recharge_stat(0.0f),
			temperature_stat(0.0f),
			gun_data() {}
		GunStatus(int m_id, float r, float h, float c, int b, const ProjectileData &pd) : // "Fast" ctor for creation
			mount_id(m_id),
			is_firing(false),
			recharge_stat(r),
			temperature_stat(0.0f),
			gun_data(r, h, c, b, pd) {}
		GunStatus(const GunStatus& gs) : // Copy ctor
			mount_id(gs.mount_id),
			is_firing(gs.is_firing),
			recharge_stat(gs.recharge_stat),
			temperature_stat(gs.temperature_stat),
			gun_data(gs.gun_data) {}
		int mount_id;
		bool is_firing;
		float recharge_stat;
		float temperature_stat;
		GunData gun_data;
	};

	std::vector<Mount> m_mounts;

	std::vector<GunStatus> m_guns;
	//TODO: mmmh... Should I put cooler PER gun?
	float m_cooler_boost;
};

#endif // FIXEDGUNS_H
