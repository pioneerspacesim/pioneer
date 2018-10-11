// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FixedGuns.h"
#include "GameSaveError.h"
#include "Beam.h"

FixedGuns::FixedGuns()
{
}

FixedGuns::~FixedGuns()
{
}

bool FixedGuns::IsFiring()
{
	bool gunstate = false;
	for (int j = 0; j < Guns::GUNMOUNT_MAX; j++)
		gunstate |= m_is_firing[j];
	return gunstate;
}

bool FixedGuns::IsFiring(const int num)
{
	return m_is_firing[num];
}

bool FixedGuns::IsBeam(const int num)
{
	return m_gun[num].projData.beam;
}

void FixedGuns::Init(DynamicBody *b)
{
	for (int i=0; i<Guns::GUNMOUNT_MAX; i++) {
		// Initialize structs
		m_is_firing[i] = false;
		m_gun[i].recharge = 0;
		m_gun[i].temp_heat_rate = 0;
		m_gun[i].temp_cool_rate = 0;
		m_gun[i].projData.lifespan = 0;
		m_gun[i].projData.damage = 0;
		m_gun[i].projData.length = 0;
		m_gun[i].projData.width = 0;
		m_gun[i].projData.mining = false;
		m_gun[i].projData.speed = 0;
		m_gun[i].projData.color = Color::BLACK;
		// Set there's no guns:
		m_gun_present[i] = false;
		m_recharge_stat[i] = 0.0;
		m_temperature_stat[i] = 0.0;
	};
	b->AddFeature( DynamicBody::FIXED_GUNS );
}

void FixedGuns::SaveToJson( Json &jsonObj, Space *space )
{

	Json gunArray = Json::array(); // Create JSON array to contain gun data.

	for (int i = 0; i<Guns::GUNMOUNT_MAX; i++)
	{
		Json gunArrayEl = Json::object(); // Create JSON object to contain gun.
		gunArrayEl["state"] = m_is_firing[i];
		gunArrayEl["recharge"] = m_recharge_stat[i];
		gunArrayEl["temperature"] = m_temperature_stat[i];
		gunArray.push_back(gunArrayEl); // Append gun object to array.
	}
	jsonObj["guns"] = gunArray; // Add gun array to ship object.
};

void FixedGuns::LoadFromJson( const Json &jsonObj, Space *space )
{
	Json gunArray = jsonObj["guns"].get<Json::array_t>();
	assert(Guns::GUNMOUNT_MAX == gunArray.size());

	try {
		for (unsigned int i = 0; i < Guns::GUNMOUNT_MAX; i++)
		{
			Json gunArrayEl = gunArray[i];

			m_is_firing[i] = gunArrayEl["state"];
			m_recharge_stat[i] = gunArrayEl["recharge"];
			m_temperature_stat[i] = gunArrayEl["temperature"];
		}
	} catch (Json::type_error &e) {
		throw SavedGameCorruptException();
	}
};

void FixedGuns::InitGun( SceneGraph::Model *m, const char *tag, int num)
{
	const SceneGraph::MatrixTransform *mt = m->FindTagByName(tag);
	if (mt) {
		const matrix4x4f &trans = mt->GetTransform();
		m_gun[num].pos = trans.GetTranslate();
		m_gun[num].dir = trans.GetOrient().VectorZ();
	} else {
		// Output("WARNING: tag %s not found for gun %i\n", tag, num);
		// XXX deprecated
		m_gun[num].pos = (num==Guns::GUN_FRONT) ? vector3f(0,0,0) : vector3f(0,0,0);
		m_gun[num].dir = (num==Guns::GUN_REAR) ? vector3f(0,0,-1) : vector3f(0,0,1);
	}
}

void FixedGuns::MountGun(const int num, const float recharge, const float lifespan, const float damage, const float length,
	const float width, const bool mining, const Color& color, const float speed, const bool beam, const float heatrate, const float coolrate )
{
	if(num >= Guns::GUNMOUNT_MAX)
		return;
	// Here we have projectile data MORE recharge time
	m_is_firing[num] = false;
	m_gun[num].recharge = recharge;
	m_gun[num].temp_heat_rate = heatrate; // TODO: More fun if you have a variable "speed" for temperature
	m_gun[num].temp_cool_rate = coolrate;
	m_gun[num].projData.lifespan = lifespan;
	m_gun[num].projData.damage = damage;
	m_gun[num].projData.length = length;
	m_gun[num].projData.width = width;
	m_gun[num].projData.mining = mining;
	m_gun[num].projData.color = color;
	m_gun[num].projData.speed = speed;
	m_gun[num].projData.beam = beam;
	m_gun_present[num] = true;
};

void FixedGuns::UnMountGun( int num )
{
	if(num >= Guns::GUNMOUNT_MAX)
		return;
	if(!m_gun_present[num])
		return;
	m_is_firing[num] = false;
	m_gun[num].recharge = 0;
	m_gun[num].temp_heat_rate = 0;
	m_gun[num].temp_cool_rate = 0;
	m_gun[num].projData.lifespan = 0;
	m_gun[num].projData.damage = 0;
	m_gun[num].projData.length = 0;
	m_gun[num].projData.width = 0;
	m_gun[num].projData.mining = false;
	m_gun[num].projData.speed = 0;
	m_gun[num].projData.color = Color::BLACK;
	m_gun_present[num] = false;
}

bool FixedGuns::Fire( const int num, Body* b )
{
	if (!m_gun_present[num]) return false;
	if (!m_is_firing[num]) return false;
	// Output("Firing gun %i, present\n", num);
	// Output(" is firing\n");
	if (m_recharge_stat[num]>0) return false;
	// Output(" recharge stat <= 0\n");
	if (m_temperature_stat[num] > 1.0) return false;
	// Output(" temperature stat <= 1.0\n");

	const vector3d dir = b->GetOrient() * vector3d(m_gun[num].dir);
	const vector3d pos = b->GetOrient() * vector3d(m_gun[num].pos) + b->GetPosition();
	const vector3d dirVel = m_gun[num].projData.speed * dir.Normalized();

	m_temperature_stat[num] += m_gun[num].temp_heat_rate;
	m_recharge_stat[num] = m_gun[num].recharge;

	if ( m_gun[num].dual )
	{
		const vector3d orient_norm = b->GetOrient().VectorY();
		const vector3d sep = 5.0 * dir.Cross(orient_norm).NormalizedSafe();
		if(m_gun[num].projData.beam)
		{
			Beam::Add(b, m_gun[num].projData, pos + sep, b->GetVelocity(), dir.Normalized());
			Beam::Add(b, m_gun[num].projData, pos - sep, b->GetVelocity(), dir.Normalized());
		}
		else
		{
			Projectile::Add(b, m_gun[num].projData, pos + sep, b->GetVelocity(), dirVel);
			Projectile::Add(b, m_gun[num].projData, pos - sep, b->GetVelocity(), dirVel);
		}
	} else {
		if(m_gun[num].projData.beam)
		{
			Beam::Add(b, m_gun[num].projData, pos, b->GetVelocity(), dir.Normalized());
		}
		else
		{
			Projectile::Add(b, m_gun[num].projData, pos, b->GetVelocity(), dirVel);
		}
	}
	return true;
};

void FixedGuns::UpdateGuns( float timeStep )
{
	for (int i=0; i<Guns::GUNMOUNT_MAX; i++) {
		if ( !m_gun_present[i] )
			continue;

		m_recharge_stat[i] -= timeStep;

		float rateCooling = m_gun[i].temp_cool_rate;
		rateCooling *= m_cooler_boost;
		m_temperature_stat[i] -= rateCooling*timeStep;

		if (m_temperature_stat[i] < 0.0f)
			m_temperature_stat[i] = 0;
		else if (m_temperature_stat[i] > 1.0f)
			m_is_firing[i] = false;

		if (m_recharge_stat[i] < 0.0f)
			m_recharge_stat[i] = 0;
	}

}
float FixedGuns::GetGunTemperature(int idx) const {
	if(m_gun_present[idx])
		return m_temperature_stat[idx];
	else
		return 0.0f;
}
