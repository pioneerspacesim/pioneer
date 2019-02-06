// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FixedGuns.h"

#include "Beam.h"
#include "DynamicBody.h"
#include "GameSaveError.h"
#include "Projectile.h"
#include "StringF.h"
#include "scenegraph/Model.h"

#pragma GCC optimize ("O0")

FixedGuns::FixedGuns(DynamicBody* b)
{
	b->AddFeature(DynamicBody::FIXED_GUNS);
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
		Json gunArrayEl = Json::object(); // Create JSON object to contain gun.
		gunArrayEl["state"] = m_guns[i].is_firing;
		gunArrayEl["recharge"] = m_guns[i].recharge_stat;
		gunArrayEl["temperature"] = m_guns[i].temperature_stat;
		// Save "GunData" of this gun:
		gunArrayEl["gd_dual"] = m_guns[i].gun_data.dual;
		gunArrayEl["gd_recharge"] = m_guns[i].gun_data.recharge;
		gunArrayEl["gd_cool_rate"] = m_guns[i].gun_data.temp_cool_rate;
		gunArrayEl["gd_heat_rate"] = m_guns[i].gun_data.temp_heat_rate;
		gunArrayEl["gd_hard_point"] = m_guns[i].gun_data.hard_point->name; // <- Save the name of hardpoint (mount)
		// Save "ProjectileData" of this gun:
		gunArrayEl["pd_beam"] = m_guns[i].gun_data.projData.beam;
		Json colorArray = Json::array();
		for (unsigned int i = 0; i < 3; i++) {
			Json arrayElem({});
			arrayElem["color"] = m_guns[i].gun_data.projData.color[i];
			colorArray.push_back(arrayElem);
		}
		gunArrayEl["pd_color"] = colorArray;
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
	Json gunArray = jsonObj["guns"].get<Json::array_t>();

	m_guns.reserve(gunArray.size());
	try {
		for (unsigned int i = 0; i < gunArray.size(); i++) {
			Json gunArrayEl = gunArray[i];
			GunStatus gs;
			// Load status:
			gs.is_firing = gunArrayEl["state"];
			gs.recharge_stat = gunArrayEl["recharge"];
			gs.temperature_stat = gunArrayEl["temperature"];
			// Load "GunData" for this gun:
			gs.gun_data.dual = gunArrayEl["gd_dual"];
			gs.gun_data.recharge = gunArrayEl["gd_recharge"];
			gs.gun_data.temp_cool_rate = gunArrayEl["gd_cool_rate"];
			gs.gun_data.temp_heat_rate = gunArrayEl["gd_heat_rate"];
			gs.gun_data.hard_point = nullptr;
			std::string mount_name; // <- Load the name of hardpoint (mount)
			mount_name = gunArrayEl["gd_hard_point"];
			for (Mount m: m_mounts) {
				if (m.name == mount_name) {
					gs.gun_data.hard_point = &m;
				}
			}
			if (gs.gun_data.hard_point == nullptr) throw SavedGameCorruptException();
			// Load "ProjectileData" for this gun:
			gs.gun_data.projData.beam = gunArrayEl["pd_beam"];
			Json colorsArray = gunArrayEl["pd_color"].get<Json::array_t>();
			for (unsigned int i = 0; i < 3; i++) {
				Json arrayElem = colorsArray[i];
				gs.gun_data.projData.color[i] = arrayElem["color"];
			}
			gs.gun_data.projData.damage = gunArrayEl["pd_damage"];
			gs.gun_data.projData.length = gunArrayEl["pd_length"];
			gs.gun_data.projData.lifespan = gunArrayEl["pd_lifespan"];
			gs.gun_data.projData.mining = gunArrayEl["pd_mining"];
			gs.gun_data.projData.speed = gunArrayEl["pd_speed"];
			gs.gun_data.projData.width = gunArrayEl["pd_width"];

			m_guns.push_back(gs);
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
};

void FixedGuns::InitGuns(SceneGraph::Model *m)
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
			// Multiple "tag" type: we should group tags with
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
		// Old "tag" type, like "tag_gunmount_0":
		// construct a Mount with a single barrel
		if (break_) continue;
		Mount mount;
		mount.name = name.substr(0,14);
		printf("\tMount[%i] = %s Dir: ", i, mount.name.c_str());
		const matrix4x4f &trans = mounts_founds[i]->GetTransform();
		mount.locs.push_back(vector3d(trans.GetTranslate()));
		mount.dir = vector3d(trans.GetOrient().VectorZ().Normalized()); /// TODO: la direzione deve essere avanti o indietro (poi magari settiamo l'angolo)
		mount.dir.Print();
		m_mounts.push_back(mount);
	}
}

void FixedGuns::MountGun(const int num, const float recharge, const float heatrate, const float coolrate, const ProjectileData &pd)
{
	// Check mount (num) is valid
	if (num >= m_mounts.size())
		return;
	// Check mount is free:
	for (int i = 0; i < m_guns.size(); i++) {
		if (m_guns[i].gun_data.hard_point == &m_mounts[num]) return;
	}
	/// Usare una "initialization list" così si evita il costruttore, cercare anche di farlo "leggibile"
	GunStatus gs;
	gs.gun_data.hard_point = &m_mounts[num];
	gs.gun_data.recharge = recharge;
	gs.gun_data.temp_heat_rate = heatrate;
	gs.gun_data.temp_cool_rate = coolrate;
	gs.gun_data.projData = pd;
	gs.recharge_stat = recharge;
	gs.temperature_stat = 0;
	gs.is_firing = false;
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
		if (m_guns[i].gun_data.hard_point == &m_mounts[num]) break;
	}
	if (i == m_guns.size()) return;
	// Mount 'i' is used and should be freed
	std::swap(m_guns[i], m_guns.back());
	m_guns.pop_back();
}

bool FixedGuns::Fire(const int num, Body *shooter)
{
	if (num >= m_guns.size()) return false;
	if (!m_guns[num].is_firing) return false;
	// Output("Firing gun %i, present\n", num);
	// Output(" is firing\n");
	if (m_guns[num].recharge_stat > 0) return false;
	// Output(" recharge stat <= 0\n");
	if (m_guns[num].temperature_stat > 1.0) return false;
	// Output(" temperature stat <= 1.0\n");

	m_guns[num].temperature_stat += m_guns[num].gun_data.temp_heat_rate;
	m_guns[num].recharge_stat = m_guns[num].gun_data.recharge;

	const int maxBarrels = std::min(size_t(m_guns[num].gun_data.dual ? 2 : 1), m_guns[num].gun_data.hard_point->locs.size());

	for (int iBarrel = 0; iBarrel < maxBarrels; iBarrel++) {
		const vector3d dir = (shooter->GetOrient() * vector3d(m_guns[num].gun_data.hard_point->dir)).Normalized();
		const vector3d pos = shooter->GetOrient() * vector3d(m_guns[num].gun_data.hard_point->locs[iBarrel]) + shooter->GetPosition();

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
	if (idx <= m_guns.size())
		return m_guns[idx].temperature_stat;
	else
		return 0.0f;
}
