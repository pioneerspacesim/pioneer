// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FixedGuns.h"

#include "Beam.h"
#include "Body.h"
#include "GameSaveError.h"
#include "Projectile.h"
#include "StringF.h"
#include "scenegraph/Model.h"
#include "scenegraph/MatrixTransform.h"
#include "JsonUtils.h"

FixedGuns::FixedGuns(Body* b)
{
	b->AddFeature(Body::FIXED_GUNS);
}

FixedGuns::~FixedGuns()
{
}

void FixedGuns::SaveToJson(Json &jsonObj, Space *space)
{
	Json gunArray = Json::array(); // Create JSON array to contain gun data.

	for (int i = 0; i < m_guns.size(); i++) {
		Json gunArrayEl({});// = Json::object(); // Create JSON object to contain gun.
		//Json projectileObj({})
		gunArrayEl["state"] = m_guns[i].is_firing;
		gunArrayEl["active"] = m_guns[i].is_active;
		gunArrayEl["recharge"] = m_guns[i].recharge_stat;
		gunArrayEl["temperature"] = m_guns[i].temperature_stat;
		gunArrayEl["contemporary_barrels"] = m_guns[i].contemporary_barrels;
		gunArrayEl["next_firing"] = m_guns[i].next_firing_barrels;
		gunArrayEl["mount_name"] = m_mounts[m_guns[i].mount_id].name; // <- Save the name of hardpoint (mount)
		// Save "GunData":
		gunArrayEl["gd_name"] = m_guns[i].gun_data.gun_name;
		gunArrayEl["gd_sound"] = m_guns[i].gun_data.sound;
		gunArrayEl["gd_barrels"] = m_guns[i].gun_data.barrels;
		gunArrayEl["gd_recharge"] = m_guns[i].gun_data.recharge;
		gunArrayEl["gd_cool_rate"] = m_guns[i].gun_data.temp_cool_rate;
		gunArrayEl["gd_heat_rate"] = m_guns[i].gun_data.temp_heat_rate;
		// Save "ProjectileData":
		gunArrayEl["proj_data"] = m_guns[i].gun_data.projData.SaveToJson();

		gunArray.push_back(gunArrayEl); // Append gun object to array.
	}
	jsonObj["guns"] = gunArray; // Add gun array to ship object.
};

void FixedGuns::LoadFromJson(const Json &jsonObj, Space *space)
{
	//Json projectileObj = jsonObj["projectile"];
	Json gunArray = jsonObj["guns"];

	m_guns.reserve(gunArray.size());
	try {
		for (unsigned int i = 0; i < gunArray.size(); i++) {
			Json gunArrayEl = gunArray[i];
			// Load status data:
			bool is_firing = gunArrayEl["state"];
			bool is_active = gunArrayEl["active"];
			float recharge_stat = gunArrayEl["recharge"];
			float temperature_stat = gunArrayEl["temperature"];
			int contemporary_barrels = gunArrayEl["contemporary_barrels"];
			int next_firing = gunArrayEl["next_firing"];
			std::string mount_name = gunArrayEl["mount_name"];
			int mount_id = -1;
			for (int i = 0; i < m_mounts.size(); i++) {
				if (m_mounts[i].name == mount_name.substr(0,14)) {
					mount_id = i;
					break;
				}
			}
			if (mount_id < 0) throw SavedGameCorruptException();
			// Load "GunData" for this gun:
			std::string name = gunArrayEl["gd_name"];
			std::string sound = gunArrayEl["gd_sound"];
			int barrels = gunArrayEl["gd_barrels"];
			float recharge = gunArrayEl["gd_recharge"];
			float temp_cool_rate = gunArrayEl["gd_cool_rate"];
			float temp_heat_rate = gunArrayEl["gd_heat_rate"];
			// Load "ProjectileData" for this gun:
			ProjectileData pd(gunArrayEl["proj_data"]);

			GunStatus gs(mount_id, name, sound, recharge, temp_heat_rate, temp_cool_rate, barrels, pd);

			gs.is_firing = is_firing;
			gs.is_active = is_active;
			gs.mount_id = mount_id;
			gs.recharge_stat = recharge_stat;
			gs.temperature_stat = temperature_stat;
			gs.contemporary_barrels = contemporary_barrels;
			gs.next_firing_barrels = next_firing;

			gs.UpdateFireModes(m_mounts[mount_id]);

			m_guns.push_back(gs);
		}
	} catch (Json::type_error &) {
		Output("Loading error in '%s'\n", __func__);
		throw SavedGameCorruptException();
	}
};

void FixedGuns::GetGunsTags(SceneGraph::Model *m)
{
	if (m == nullptr) {
		Output("In FixedGuns::GetGunsTags:\nNo Model no Guns, sorry...\n");
		abort();
	}
	m_mounts = m->GetGunTags();
}

bool FixedGuns::MountGun(MountId num, const std::string &name, const std::string &sound, float recharge, float heatrate, float coolrate, int barrels, const ProjectileData &pd)
{
	//printf("FixedGuns::MountGun '%s' in '%s',num: %i (Mounts %ld, guns %ld)\n", name.c_str(), m_mounts[num].name.c_str(), num, long(m_mounts.size()), long(m_guns.size()));
	// Check mount (num) is valid
	if (num >= m_mounts.size()) {
		Output("Attempt to mount a gun in %i, which is out of bounds\n", num);
		return false;
	}
	// Check ... well, there's a needs for explanations?
	if (barrels == 0) {
		Output("Attempt to mount a gun with zero barrels\n");
		return false;
	}

	// Check mount is free:
	for (int i = 0; i < m_guns.size(); i++) {
		if (m_guns[i].mount_id < 0) {
			printf("hard_point is negative!?!?\n");
			abort();
		}
		if (m_mounts[m_guns[i].mount_id].name == m_mounts[num].name.substr(0,14)) {
			Output("Attempt to mount gun %i on '%s', which is already used\n", num, m_mounts[num].name.c_str());
			return false;
		}
	}
	if (barrels > m_mounts[num].locs.size()) {
		Output("Gun with %i barrels mounted on '%s', which is for %i barrels\n", barrels, m_mounts[num].name.c_str(), int(m_mounts[num].locs.size()));
	}
	GunStatus gs(num, name, sound, recharge, heatrate, coolrate, barrels, pd);
	gs.UpdateFireModes(m_mounts[num]);

	m_guns.push_back(gs);

	return true;
};

bool FixedGuns::UnMountGun(MountId num)
{
	// Check mount (num) is valid
	if (num >= m_guns.size() && m_guns.size() > 0) {
		Output("Mount identifier (%i) is out of bounds (max is %lu) in 'UnMountGun'\n", num, m_mounts.size());
		return false;
	}
	// Check mount is used
	std::vector<GunStatus>::iterator found = std::find_if(begin(m_guns), end(m_guns), [&num](const GunStatus &gs)
	{
		return (num == gs.mount_id);
	});

	if (found == m_guns.end()) {
		Output("No gun found for the given identifier\n");
		return false;
	}
	//Output("Remove guns %i, mounted on '%s'\n", mount, m_mounts[m_guns[mount].mount_id].name.c_str());
	// Mount 'num' is used and should be freed
	std::vector<GunStatus>::iterator last = m_guns.end() - 1;
	std::swap(found, last);
	m_guns.pop_back();
	return true;
}

void FixedGuns::SetGunsFiringState(GunDir dir, int s)
{
	std::for_each(begin(m_guns), end(m_guns), [&](GunStatus &gs) { if (m_mounts[gs.mount_id].dir == dir ) gs.is_firing = s;});
}

bool FixedGuns::Fire(GunId num, Body *shooter)
{
	if (num >= m_guns.size()) return false;

	GunStatus &gun = m_guns[num];
	if (!gun.is_firing) return false;
	if (!gun.is_active) return false;
	if (gun.recharge_stat > 0) return false;
	if (gun.temperature_stat > 1.0) return false;

	gun.temperature_stat += gun.gun_data.temp_heat_rate * (float(gun.contemporary_barrels) / gun.gun_data.barrels);
	gun.recharge_stat = gun.gun_data.recharge * (float(gun.contemporary_barrels) / gun.gun_data.barrels);

	const Mount &mount = m_mounts[gun.mount_id];

	for (int iBarrel : GetFiringBarrels(gun.gun_data.barrels, gun.contemporary_barrels, gun.next_firing_barrels)) {
		// (0,0,-1) => Front ; (0,0,1) => Rear
		const vector3d front_rear = (mount.dir == GunDir::GUN_FRONT ? vector3d(0., 0., -1.) : vector3d(0., 0., 1.));
		const vector3d dir = (shooter->GetOrient() * front_rear);

		const vector3d pos = shooter->GetOrient() * vector3d(mount.locs[iBarrel]) + shooter->GetPosition();

		if (gun.gun_data.projData.beam) {
			Beam::Add(shooter, gun.gun_data.projData, pos, shooter->GetVelocity(), dir);
		} else {
			const vector3d dirVel = gun.gun_data.projData.speed * dir;
			Projectile::Add(shooter, gun.gun_data.projData, pos, shooter->GetVelocity(), dirVel);
		}
	}

	return true;
};

bool FixedGuns::UpdateGuns(float timeStep, Body *shooter)
{
	for (int i = 0; i < m_guns.size(); i++) {

		float rateCooling = m_guns[i].gun_data.temp_cool_rate;
		rateCooling *= m_cooler_boost;
		m_guns[i].temperature_stat -= rateCooling * timeStep;

		if (m_guns[i].temperature_stat < 0.0f)
			m_guns[i].temperature_stat = 0;
		else if (m_guns[i].temperature_stat > 1.0f)
			m_guns[i].is_firing = false;

		m_guns[i].recharge_stat -= timeStep;
		if (m_guns[i].recharge_stat < 0.0f)
			m_guns[i].recharge_stat = 0;
	}

	bool fire = false;

	for (GunId i = 0; i < m_guns.size(); i++) {
		bool res = Fire(i, shooter);
		// Skip sound management if 'sound' is not defined
		if (m_guns[i].gun_data.sound.empty()) continue;
		if (res) {
			fire = true;
			if (IsBeam(i)) {
				float vl, vr;
				Sound::CalculateStereo(shooter, 1.0f, &vl, &vr);
				m_guns[i].sound.Play(m_guns[i].gun_data.sound.c_str(), vl, vr, Sound::OP_REPEAT);
			} else {
				Sound::BodyMakeNoise(shooter, m_guns[i].gun_data.sound.c_str(), 1.0f);
			}
		}

		if (res && IsBeam(i)) {
			float vl, vr;
			Sound::CalculateStereo(shooter, 1.0f, &vl, &vr);
			if (!m_guns[i].sound.IsPlaying()) {
				m_guns[i].sound.Play(m_guns[i].gun_data.sound.c_str(), vl, vr, Sound::OP_REPEAT);
			} else {
				// update volume
				m_guns[i].sound.SetVolume(vl, vr);
			}
		} else if (!IsFiring(i) && m_guns[i].sound.IsPlaying()) {
			m_guns[i].sound.Stop();
		}
	}
	return fire;
}

MountId FixedGuns::FindFirstEmptyMount() const
{
	std::vector<int> free = FindEmptyMounts();
	if (free.empty()) return -1;
	else return free[0];
}

std::vector<MountId> FixedGuns::FindEmptyMounts() const
{
	std::vector<int> occupied;

	if (GetFreeMountsSize() == 0) return occupied;

	occupied.reserve(m_guns.size());

	std::for_each(begin(m_guns), end(m_guns), [&occupied](const GunStatus &gs) // <- Sure there's a better alghorithm
	{
		if (gs.mount_id >= 0) occupied.emplace_back(gs.mount_id);
	});

	std::sort(begin(occupied), end(occupied));

	std::vector<int> free;
	free.reserve(m_mounts.size() - occupied.size());

	for (int mount = 0; mount < m_mounts.size(); mount++) {
		if (!std::binary_search(begin(occupied), end(occupied), mount)) {
			free.push_back(mount);
		}
	}
	return free;
}

MountId FixedGuns::FindMountOfGun(const std::string &name) const
{
	std::vector<GunStatus>::const_iterator found = std::find_if(begin(m_guns), end(m_guns), [&name](const GunStatus &gs)
	{
		if (gs.gun_data.gun_name == name) {
			return true;
		};
	});
	if (found != m_guns.end()) {
		return (*found).mount_id;
	}
	return -1;
}

void FixedGuns::SetActivationStateOfGun(GunId num, bool active)
{
	if (num < m_guns.size())
		m_guns[num].is_active = active;
	else {
		Output("Given gun identifier (%i) is out of bounds (max is %i)\n", num, int(m_guns.size()));
	}
}

bool FixedGuns::GetActivationStateOfGun(GunId num) const
{
	if (num < m_guns.size())
		return m_guns[num].is_active;
	else {
		Output("Given gun identifier (%i) is out of bounds (max is %lu)\n", num, m_guns.size());
		return false;
	}
}

int FixedGuns::GetNumAvailableBarrels(GunId num)
{
	if (num < m_guns.size())
		return std::min(m_guns[num].gun_data.barrels, unsigned(m_mounts[m_guns[num].mount_id].locs.size()));
	else {
		Output("Given gun identifier (%i) is out of bounds (max is %lu)\n", num, m_guns.size());
 		return false;
 	}
}

int FixedGuns::GetNumBarrels(GunId num)
{
	if (num < m_guns.size())
		return m_guns[num].gun_data.barrels;
	else {
		Output("Given gun identifier (%i) is out of bounds (max is %lu)\n", num, m_guns.size());
		return false;
	}
}

int FixedGuns::GetNumActiveBarrels(GunId num)
{
	if (num < m_guns.size())
		return m_guns[num].contemporary_barrels;
	else {
		Output("Given gun identifier (%i) is out of bounds (max is %lu)\n", num, m_guns.size());
		return false;
	}
}

void FixedGuns::CycleFireModeForGun(GunId num)
{
	if (num < m_guns.size()) {
		std::vector<int> &fire_modes = m_guns[num].fire_modes;
		std::vector<int>::iterator it = std::find(begin(fire_modes), end(fire_modes), m_guns[num].contemporary_barrels);

		if (it == fire_modes.end()) {
			printf("What?! In FixedGuns::CycleFireModeForGun seems actual fire mode don't exist...\n");
			abort();
		}
		it++;
		if (it == fire_modes.end())
			m_guns[num].contemporary_barrels = 1;
		else m_guns[num].contemporary_barrels = (*it);
		//printf("FixedGuns::CycleFireModeForGun %i, set to %i\n", num, m_guns[num].contemporary_barrels);
	} else {
		Output("Given gun identifier (%i) is out of bounds (max is %lu)\n", num, m_guns.size());
		return;
	}
}

GunDir FixedGuns::IsFront(GunId num) const
{
	if (num < m_guns.size())
		return m_mounts[m_guns[num].mount_id].dir;
	else
		return GunDir::GUNMOUNT_MAX;
}

bool FixedGuns::IsFiring() const
{
	bool gunstate = false;
	for (int j = 0; j < m_guns.size(); j++)
		gunstate |= m_guns[j].is_firing;
	return gunstate;
}

bool FixedGuns::IsFiring(GunId num) const
{
	return m_guns[num].is_firing;
}

bool FixedGuns::IsBeam(GunId num) const
{
	return m_guns[num].gun_data.projData.beam;
}

float FixedGuns::GetGunTemperature(GunId idx) const
{
	if (idx < m_guns.size())
		return m_guns[idx].temperature_stat;
	else
		return 0.0f;
}

std::vector<int> FixedGuns::GetFiringBarrels(int max_barrels, int contemporary_barrels, int &actual_barrel)
{
	std::vector<int> cont;
	cont.reserve(contemporary_barrels);

	for (int i = actual_barrel; i < contemporary_barrels + actual_barrel; i ++)
		cont.push_back(i);

	actual_barrel += contemporary_barrels;
	if (actual_barrel >= max_barrels) actual_barrel = 0;
	return cont;
}

std::vector<int> FixedGuns::CalculateFireModes(int b)
{
	std::vector<int> fire_modes;
	for (int factor = 1; factor <= b/2; factor++)
		if (b % factor == 0) fire_modes.push_back(factor);
	fire_modes.push_back(b);

	return fire_modes;
}

void FixedGuns::GunStatus::UpdateFireModes(const Mount &mount)
{
	int barrels = std::min(gun_data.barrels, unsigned(mount.locs.size()));
	//printf("UpdateFireModes %u & %lu\n", gun_data.barrels, mount.locs.size());
	fire_modes = FixedGuns::CalculateFireModes(barrels);
}
