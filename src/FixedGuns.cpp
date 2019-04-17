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
		gunArrayEl["recharge"] = m_guns[i].recharge_stat;
		gunArrayEl["temperature"] = m_guns[i].temperature_stat;
		gunArrayEl["mount_name"] = m_mounts[m_guns[i].mount_id].name; // <- Save the name of hardpoint (mount)
		// Save "GunData":
		gunArrayEl["gd_name"] = m_guns[i].gun_data.gun_name;
		gunArrayEl["gd_barrels"] = m_guns[i].gun_data.barrels;
		gunArrayEl["gd_recharge"] = m_guns[i].gun_data.recharge;
		gunArrayEl["gd_cool_rate"] = m_guns[i].gun_data.temp_cool_rate;
		gunArrayEl["gd_heat_rate"] = m_guns[i].gun_data.temp_heat_rate;
		// Save "ProjectileData":
		gunArrayEl["pd_beam"] = m_guns[i].gun_data.projData.beam;
		gunArrayEl["pd_color"] = m_guns[i].gun_data.projData.color;
		gunArrayEl["pd_damage"] = m_guns[i].gun_data.projData.damage;
		gunArrayEl["pd_length"] = m_guns[i].gun_data.projData.length;
		gunArrayEl["pd_lifespan"] = m_guns[i].gun_data.projData.lifespan;
		gunArrayEl["pd_mining"] = m_guns[i].gun_data.projData.mining;
		gunArrayEl["pd_speed"] = m_guns[i].gun_data.projData.speed;
		gunArrayEl["pd_width"] = m_guns[i].gun_data.projData.width;

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
			float is_firing = gunArrayEl["state"];
			float recharge_stat = gunArrayEl["recharge"];
			float temperature_stat = gunArrayEl["temperature"];
			std::string mount_name; // <- Load the name of hardpoint (mount)
			int mount_id = -1;
			mount_name = gunArrayEl["mount_name"];
			for (int i = 0; i < m_mounts.size(); i++) {
				if (m_mounts[i].name == mount_name.substr(0,14)) {
					mount_id = i;
					break;
				}
			}
			if (mount_id < 0) throw SavedGameCorruptException();
			// Load "GunData" for this gun:
			std::string name = gunArrayEl["gd_name"];
			int barrels = gunArrayEl["gd_barrels"];
			float recharge = gunArrayEl["gd_recharge"];
			float temp_cool_rate = gunArrayEl["gd_cool_rate"];
			float temp_heat_rate = gunArrayEl["gd_heat_rate"];
			// Load "ProjectileData" for this gun:
			ProjectileData pd;

			pd.beam = gunArrayEl["pd_beam"];
			pd.color = gunArrayEl["pd_color"];
			pd.damage = gunArrayEl["pd_damage"];
			pd.length = gunArrayEl["pd_length"];
			pd.lifespan = gunArrayEl["pd_lifespan"];
			pd.mining = gunArrayEl["pd_mining"];
			pd.speed = gunArrayEl["pd_speed"];
			pd.width = gunArrayEl["pd_width"];

			GunStatus gs(mount_id, name, recharge, temp_heat_rate, temp_cool_rate, barrels, pd);

			gs.is_firing = is_firing;
			gs.mount_id = mount_id;
			gs.recharge_stat = recharge_stat;
			gs.temperature_stat = temperature_stat;

			m_guns.push_back(gs);
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
};

void FixedGuns::ParseModelTags(SceneGraph::Model *m)
{
	const std::string test = "tag_gunmount";
	SceneGraph::Model::TVecMT mounts_founds;
	m->FindTagsByStartOfName(test, mounts_founds);

	m_mounts.clear();
	m_mounts.reserve(mounts_founds.size());

	bool break_; // <- Used to "break" from inner 'for' cycle
	//printf("Model name= '%s'; Tags n°= %u\n", m->GetName().c_str(), int(mounts_founds.size()));

	for (int i = 0; i < mounts_founds.size(); i++) {
		break_ = false;
		const std::string &name = mounts_founds[i]->GetName();
		if (name.length() > 14) {
			// Multiple "tag" type: we group tags with
			// the same index
			std::string name_to_first_index = name.substr(0,14);
			for (int j = 0; j < m_mounts.size(); j++ )
				// Check we already have this gun
				if (m_mounts[j].name.substr(0,14) == name_to_first_index) {
					// Add a barrel
					//printf("Add a barrel\n");
					const matrix4x4f &trans = mounts_founds[i]->GetTransform();
					m_mounts[j].locs.push_back(vector3d(trans.GetTranslate()));
					break_ = true;
					break;
			}
		}
		// Old "tag" type, like "tag_gunmount_0",
		// or another barrel for an already present
		// gun.
		if (break_) continue;
		Mount mount;
		mount.name = name.substr(0,14);
		const matrix4x4f &trans = mounts_founds[i]->GetTransform();
		mount.locs.push_back(vector3d(trans.GetTranslate()));
		mount.dir = vector3d(trans.GetOrient().VectorZ().Normalized()); /// TODO: la direzione deve essere avanti o indietro (poi magari settiamo l'angolo)
		m_mounts.push_back(mount);
	}
	// TODO LONG TERM: find and fetch data from ShipType
	// about a possible 'size' of this mount, or if this
	// mount is 'external' (gun is visible) or not...
	m_mounts.shrink_to_fit();
}

bool FixedGuns::MountGun(const int num, const std::string &name, const float recharge, const float heatrate, const float coolrate, const int barrels, const ProjectileData &pd)
{
	//printf("FixedGuns::MountGun in '%s',num: %i (Mounts %ld, guns %ld)\n", m_mounts[num].name.c_str(), num, long(m_mounts.size()), long(m_guns.size()));
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
	GunStatus gs(num, name, recharge, heatrate, coolrate, barrels, pd);
	m_guns.push_back(gs);
	return true;
};

bool FixedGuns::UnMountGun(int num)
{
	// Check mount (num) is valid
	if (num >= m_mounts.size()) {
		Output("Mount identifier (%i) is out of bounds in 'UnMountGun'\n", num);
		return false;
	}
	// Check mount is used
	int mount = 0;
	for (; mount < m_guns.size(); mount++) {
		if (m_guns[mount].mount_id == num) break;
	}
	if (mount == m_guns.size()) {
		Output("No gun found for the given identifier\n");
		return false;
	}
	//Output("Remove guns %i, mounted on '%s'\n", mount, m_mounts[m_guns[mount].mount_id].name.c_str());
	// Mount 'i' is used and should be freed
	std::swap(m_guns[mount], m_guns.back());
	m_guns.pop_back();
	return true;
}

void FixedGuns::SetGunFiringState(int idx, int s)
{
	// TODO: Handle "idx", which is the direction (front or rear)
	std::for_each(begin(m_guns), end(m_guns), [&s](GunStatus &gs) { gs.is_firing = s;});
}

bool FixedGuns::Fire(const int num, const Body *shooter)
{
	if (num >= m_guns.size()) return false;
	if (!m_guns[num].is_firing) return false;
	if (m_guns[num].recharge_stat > 0) return false;
	if (m_guns[num].temperature_stat > 1.0) return false;

	m_guns[num].temperature_stat += m_guns[num].gun_data.temp_heat_rate;
	m_guns[num].recharge_stat = m_guns[num].gun_data.recharge;

	const Mount &mount = m_mounts[m_guns[num].mount_id];

	const int maxBarrels = std::min(size_t(m_guns[num].gun_data.barrels ? 2 : 1), mount.locs.size());

	for (int iBarrel = 0; iBarrel < maxBarrels; iBarrel++) {
		const vector3d dir = (shooter->GetOrient() * vector3d(mount.dir)).Normalized();
		const vector3d pos = shooter->GetOrient() * vector3d(mount.locs[iBarrel]) + shooter->GetPosition();

		if (m_guns[num].gun_data.projData.beam) {
			Beam::Add(shooter, m_guns[num].gun_data.projData, pos, shooter->GetVelocity(), dir);
		} else {
			const vector3d dirVel = m_guns[num].gun_data.projData.speed * dir;
			Projectile::Add(shooter, m_guns[num].gun_data.projData, pos, shooter->GetVelocity(), dirVel);
		}
	}

	return true;
};

void FixedGuns::UpdateGuns(float timeStep)
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
}

int FixedGuns::FindFirstEmptyMount() const
{
	std::vector<int> free = FindEmptyMounts();
	if (free.empty()) return -1;
	else return free[0];
}

std::vector<int> FixedGuns::FindEmptyMounts() const
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

int FixedGuns::FindMountOfGun(const std::string &name) const
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

bool FixedGuns::IsFiring() const
{
	bool gunstate = false;
	for (int j = 0; j < m_guns.size(); j++)
		gunstate |= m_guns[j].is_firing;
	return gunstate;
}

bool FixedGuns::IsFiring(const int num) const
{
	return m_guns[num].is_firing;
}

bool FixedGuns::IsBeam(const int num) const
{
	return m_guns[num].gun_data.projData.beam;
}

float FixedGuns::GetGunTemperature(int idx) const
{
	if (idx < m_guns.size())
		return m_guns[idx].temperature_stat;
	else
		return 0.0f;
}
