// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "Json.h"
#include "Projectile.h"
#include "vector3.h"

class DynamicBody;
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
	FixedGuns(DynamicBody *b);
	~FixedGuns();
	void InitGuns(SceneGraph::Model *m);
	void UpdateGuns(float timeStep);
	bool Fire(const int num, Body *b);
	int GetGunsNumber() const { return int(m_guns.size()); }
	bool IsFiring();
	bool IsFiring(const int num);
	bool IsBeam(const int num);
	float GetGunTemperature(int idx) const;
	inline void IsDual(int idx, bool dual) { m_guns[idx].gun_data.dual = dual; };
	void MountGun(const int num, const float recharge, const float heatrate, const float coolrate, const ProjectileData &pd);
	void UnMountGun(int num);
	inline float GetGunRange(int idx) { return m_guns[idx].gun_data.projData.speed * m_guns[idx].gun_data.projData.lifespan; };
	inline float GetProjSpeed(int idx) { return m_guns[idx].gun_data.projData.speed; };
	inline void SetCoolingBoost(float cooler) { m_cooler_boost = cooler; };
	inline void SetGunFiringState(int idx, int s)
	{
		if (idx < m_guns.size())
			m_guns[idx].is_firing = s;
	};
	void SaveToJson(Json &jsonObj, Space *space);
	void LoadFromJson(const Json &jsonObj, Space *space);

private:
	// Structure holding name, position and direction of a mount (coming from Model data)
	struct Mount {
		std::string name;
		std::vector<vector3d> locs;
		vector3d dir;
	};

	// Structure holding data of a single (maybe with multiple barrels) 'mounted' gun.
	struct GunData {
		Mount *hard_point;
		float recharge;
		float temp_heat_rate;
		float temp_cool_rate;
		bool dual;
		ProjectileData projData;
	};

	// Structure holding actual status of a gun
	struct GunStatus {
		GunStatus()  :
		is_firing(false),
		recharge_stat(0.0f),
		temperature_stat(0.0f) {}
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
