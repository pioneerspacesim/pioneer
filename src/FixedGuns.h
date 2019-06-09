// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "JsonFwd.h"
#include "ProjectileData.h"
#include "vector3.h"
#include "sound/Sound.h"
#include "scenegraph/Mount.h"

class Body;
class Space;

namespace SceneGraph {
	class Model;
}

typedef int GunId;
typedef int MountId;

/*
	TODO1:
	bool SwapTwoMountedGuns(int gun_a, int gun_b);

	TODO2:
	in Lua:
	1) handle a simple swap in info's view,
	2) put some time in it and allow it only if in a station
	(maybe allow it only if you have a crew?)

	TODO3: mmmh... Should cooler be PER gun?
	...And should customization of mounted guns possible?

	TODO LONG TERM: find and fetch data from ShipType
	about a possible 'size' of this mount, or if this
	mount is 'external' (gun is visible) or not...
*/


class FixedGuns {
public:
	FixedGuns() = delete;
	FixedGuns(Body *b);
	~FixedGuns();

	void SaveToJson(Json &jsonObj, Space *space);
	void LoadFromJson(const Json &jsonObj, Space *space);

	/* Reset&copy Mounts from a model
	*/
	void GetGunsTags(SceneGraph::Model *m);

	/* Mount a gun in "num", given its datas,
	 / will return true if succesfully mount
	 / the gun (ex. mount is valid and free)
	*/
	bool MountGun(MountId num, const std::string &name, const std::string &sound, float recharge, float heatrate, float coolrate, int barrels, const ProjectileData &pd);
	/* Unmount a gun from a mount point,
	 / will return true if mount is occupied
	*/
	bool UnMountGun(MountId num);

	/* Set guns firing in the given direction
	 / when "s" is not false
	*/
	void SetGunsFiringState(GunDir dir, int s);

	// Return value is 'true' whenever a new projectile is fired
	bool UpdateGuns(float timeStep, Body *shooter);

	int GetMountsSize() const { return int(m_mounts.size()); };
	int GetMountedGunsNum() const { return int(m_guns.size()); }
	int GetFreeMountsSize() const { return int(m_mounts.size() - m_guns.size()); };
	MountId FindFirstEmptyMount() const;
	std::vector<MountId> FindEmptyMounts() const;
	MountId FindMountOfGun(const std::string &name) const;

	/* Set or inquire the GunId abour its activation state
	*/
	void SetActivationStateOfGun(GunId num, bool active);
	bool GetActivationStateOfGun(GunId num) const;

	/* return the number of barrels usable,
	 / which is the minimum of barrels of gun
	 / and barrels on mount
	*/
	int GetNumAvailableBarrels(GunId num);

	/* return the number of barrels of
	 / the gun
	*/

	int GetNumBarrels(GunId num);
	/* Given a valid gun identifier,
	 / return the number of barrels which
	 / will fire when ready
	*/
	int GetNumActiveBarrels(GunId num);

	/* Given a GunId, change the number of barrels
	 / which will be fired simultaneously
	 / Eg: if barrels are 4, cycle through firing
	 / 1, 2 or 4 barrels togheter
	*/
	void CycleFireModeForGun(GunId num);

	GunDir IsFront(GunId num) const;
	bool IsFiring() const;
	bool IsFiring(GunId num) const;
	bool IsBeam(GunId num) const;
	float GetGunTemperature(GunId idx) const;
	const std::string &GetGunName(GunId idx) const { return m_guns[idx].gun_data.gun_name; };
	inline float GetGunRange(GunId idx) const { return m_guns[idx].gun_data.projData.speed * m_guns[idx].gun_data.projData.lifespan; };
	inline float GetProjSpeed(GunId idx) const { return m_guns[idx].gun_data.projData.speed; };
	inline void SetCoolingBoost(float cooler) { m_cooler_boost = cooler; };

private:

	/* Fire given GunId given the shooter
	 / Just a split for "historical" reasons and in order
	 / to avoid a big function for updating
	*/
	bool Fire(GunId num, Body *shooter);

	/* Get a list of barrels which can fire
	 / given the fire mode
	 / Eg: if barrels are 4 and fire mode ask
	 / for 2 barrels simoultaneously firing,
	 / return (1,2) and (3,4) given actual_barrel
	 / which is incremented.
	*/
	static std::vector<int> GetFiringBarrels(int max_barrels, int contemporary_barrels, int &actual_barrel);

	/* Given a number will return their factors,
	 / which in turn are the number of barrels
	 / which can be fired simoultaneusly
	*/
	static std::vector<int> CalculateFireModes(int num_barrels);

	// Structure holding data of a single (maybe with multiple barrel) 'mounted' gun.
	struct GunData {
		GunData() = delete;
		GunData(const std::string &n, const std::string &s, float r, float h, float c, unsigned b, const ProjectileData &pd) : // "Faster" ctor
			gun_name(n),
			sound(s),
			recharge(r),
			temp_cool_rate(c),
			temp_heat_rate(h),
			barrels(b),
			projData(pd)
			{}
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
		unsigned barrels;
		ProjectileData projData;
	};

	// Structure holding actual status of a gun
	struct GunStatus {
		GunStatus() = delete;
		GunStatus(MountId m_id, const std::string &n, const std::string &s, float r, float h, float c, unsigned b, const ProjectileData &pd) : // "Fast" ctor for creation
			mount_id(m_id),
			is_firing(false),
			is_active(true),
			recharge_stat(r),
			temperature_stat(0.0f),
			contemporary_barrels(1),
			next_firing_barrels(0),
			gun_data(n, s, r, h, c, b, pd) {}
		GunStatus(const GunStatus& gs) : // Copy ctor
			mount_id(gs.mount_id),
			is_firing(gs.is_firing),
			is_active(gs.is_active),
			recharge_stat(gs.recharge_stat),
			temperature_stat(gs.temperature_stat),
			contemporary_barrels(gs.contemporary_barrels),
			next_firing_barrels(gs.next_firing_barrels),
			fire_modes(gs.fire_modes),
			gun_data(gs.gun_data) {}
		MountId mount_id; // <- store the mount sequential number
					  //    or, if equal to -1, the fact that
					  // this gun is "invalid": it should have
					  // been deleted before... But never say
					  // never: this could be used to signal
					  // a broken or damaged gun
		bool is_firing; // <- if this gun will fire when ready
		bool is_active; // <- if this gun is active
		float recharge_stat;
		float temperature_stat;
		int contemporary_barrels; // <- the number of active barrels
		int next_firing_barrels; // <-the next group of barrels which will fire
		Sound::Event sound;
		GunData gun_data;
		std::vector<int> fire_modes;
		void UpdateFireModes(const Mount &mount);
	};

	// Vector with mounts (data coming from models)
	GunMounts m_mounts;

	// Vector with mounted guns and their status
	std::vector<GunStatus> m_guns;

	float m_cooler_boost;
};

#endif // FIXEDGUNS_H
