// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "JsonFwd.h"
#include "Projectile.h"
#include "scenegraph/Model.h"
#include "vector3.h"

class DynamicBody;
class Space;

enum Guns {
	GUN_FRONT,
	GUN_REAR,
	GUNMOUNT_MAX = 2
};

class FixedGuns : public RefCounted {
public:
	FixedGuns();
	virtual ~FixedGuns();
	void Init(DynamicBody *b);
	void InitGuns(SceneGraph::Model *m);

	virtual void SaveToJson(Json &jsonObj, Space *space);
	virtual void LoadFromJson(const Json &jsonObj, Space *space);

	void UpdateGuns(float timeStep);
	void UpdateLead(float timeStep, int num, Body *ship, Body *target);
	bool Fire(int num, Body *ship);
	void SetGunFiringState(int idx, int s);
	void SetShouldUseLeadCalc(bool enable) { m_shouldUseLeadCalc = enable; }

	bool IsFiring();
	bool IsFiring(const int num);
	bool IsBeam(const int num);
	inline void IsDual(int idx, bool dual) { m_gun[idx].dual = dual; };

	void MountGun(const int num, const float recharge, const float lifespan, const float damage, const float length,
		const float width, const bool mining, const Color &color, const float speed, const bool beam, const float heatrate, const float coolrate);
	void UnMountGun(int num);

	float GetGunTemperature(int idx) const;
	inline bool IsGunMounted(int idx) const { return m_gun_present[idx]; }
	inline float GetGunRange(int idx) const { return m_gun[idx].projData.speed * m_gun[idx].projData.lifespan; };
	inline float GetProjSpeed(int idx) const { return m_gun[idx].projData.speed; };
	inline const vector3d &GetTargetLeadPos() const { return m_targetLeadPos; }
	inline const vector3d &GetCurrentLeadDir() const { return m_currentLeadDir; }
	inline bool IsFiringSolutionOk() const { return m_firingSolutionOk; }
	inline void SetCoolingBoost(float cooler) { m_cooler_boost = cooler; };

private:
	struct GunData {
		struct GunLoc {
			vector3d pos;
			vector3d dir;
		};
		std::vector<GunLoc> locs;
		float recharge;
		float temp_heat_rate;
		float temp_cool_rate;
		bool dual;
		ProjectileData projData;
	};

	bool m_is_firing[Guns::GUNMOUNT_MAX];
	float m_recharge_stat[Guns::GUNMOUNT_MAX];
	float m_temperature_stat[Guns::GUNMOUNT_MAX];
	//TODO: Make it a vector and rework struct Gun to have bool dir={Forward,Backward}
	bool m_gun_present[Guns::GUNMOUNT_MAX];
	GunData m_gun[Guns::GUNMOUNT_MAX];
	float m_cooler_boost;
	bool m_shouldUseLeadCalc = false;
	vector3d m_targetLeadPos;
	vector3d m_currentLeadDir;
	bool m_firingSolutionOk = false;
};

#endif // FIXEDGUNS_H
