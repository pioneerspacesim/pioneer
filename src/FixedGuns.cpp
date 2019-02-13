// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FixedGuns.h"

#include "vector3.h"

#include "Beam.h"
#include "DynamicBody.h"
#include "GameSaveError.h"
#include "Projectile.h"
#include "StringF.h"
#include "scenegraph/Model.h"
#include "JsonUtils.h"

#pragma GCC optimize ("O0")

FixedGuns::FixedGuns(Body* b)
{
	b->AddFeature(Body::FIXED_GUNS);
}

FixedGuns::~FixedGuns()
{
}

bool FixedGuns::IsFiring()
{
	bool gunstate = false;
	for (int j = 0; j < m_guns.size(); j++)
		gunstate |= m_guns[j].is_firing;
	return gunstate;
}

bool FixedGuns::IsFiring(const int num)
{
	return m_guns[num].is_firing;
}

bool FixedGuns::IsBeam(const int num)
{
	return m_guns[num].gun_data.projData.beam;
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

			GunStatus gs(mount_id, recharge, temp_heat_rate, temp_cool_rate, barrels, pd);

			gs.is_firing = is_firing;
			gs.mount_id = mount_id;
			gs.recharge_stat = recharge_stat;
			gs.temperature_stat = temperature_stat;

			m_guns.push_back(gs);
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
	printf("Loaded %i guns:\n", int(m_guns.size()));
	for (int i = 0; i < m_guns.size(); i++) {
		printf("\t'%s', which dir & pos are:\n", m_mounts[m_guns[i].mount_id].name.c_str());
		printf("\t"); m_mounts[m_guns[i].mount_id].dir.Print();
		printf("\t"); m_mounts[m_guns[i].mount_id].locs[0].Print();
	}

};

void FixedGuns::ParseModelTags(SceneGraph::Model *m)
{
	m_mounts.clear();
	m_mounts.reserve(10);
	const std::string test = "tag_gunmount";
	SceneGraph::Model::TVecMT mounts_founds;
	m->FindTagsByStartOfName(test, mounts_founds);

	bool break_; // <- Used to "break" from inner 'for' cycle
	printf("Model name= '%s'; Tags n°= %u\n", m->GetName().c_str(), int(mounts_founds.size()));

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

	for (int i = 0; i < m_mounts.size(); i++) {
		printf("  Mount[%i] = %s, %i barrel, dir: ", i, m_mounts[i].name.c_str(), int(m_mounts[i].locs.size()));
		m_mounts[i].dir.Print();
	}
}

void FixedGuns::MountGun(const int num, const float recharge, const float heatrate, const float coolrate, const int barrels, const ProjectileData &pd)
{
	printf("FixedGuns::MountGun Num: %i (Mounts %ld, guns %ld)\n", num, long(m_mounts.size()), long(m_guns.size()));
	// Check mount (num) is valid
	if (num >= m_mounts.size())
		return;
	// Check ... well, there's a needs for explanations?
	if (barrels == 0) {
		Output("Attempt to mount a gun with zero barrels\n");
		return;
	}

	// Check mount is free:
	for (int i = 0; i < m_guns.size(); i++) {
		if (m_guns[i].mount_id < 0) {
			printf("hard_point is negative!?!?\n");
			abort();
		}
		printf("%s vs %s\n", m_mounts[m_guns[i].mount_id].name.c_str(),  m_mounts[num].name.substr(0,14).c_str());
		if (m_mounts[m_guns[i].mount_id].name == m_mounts[num].name.substr(0,14)) {
			Output("Attempt to mount gun %i on '%s', which is already used\n", num, m_mounts[num].name.c_str());
			return;
		}
	}
	if (barrels >= m_mounts[num].locs.size()) {
		Output("Attempt to mount a gun with %i barrels on '%s', which is for %i barrels\n", barrels, m_mounts[num].name.c_str(), int(m_mounts[num].locs.size()));
	}
	GunStatus gs(num, recharge, heatrate, coolrate, barrels, pd);
	m_guns.push_back(gs);
};

void FixedGuns::UnMountGun(int num)
{
	// Check mount (num) is valid
	if (num >= m_mounts.size())
		return;
	// Check mount is used
	int i;
	for (i = 0; i < m_guns.size(); i++) {
		if (m_guns[i].mount_id == num) break;
	}
	if (i == m_guns.size()) return;
	Output("Remove guns %i, mounted on '%s'\n", i, m_mounts[m_guns[i].mount_id].name.c_str());
	// Mount 'i' is used and should be freed
	std::swap(m_guns[i], m_guns.back());
	m_guns.pop_back();
}

bool FixedGuns::Fire(const int num, Body *shooter)
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

float FixedGuns::GetGunTemperature(int idx) const
{
	if (idx < m_guns.size())
		return m_guns[idx].temperature_stat;
	else
		return 0.0f;
}
