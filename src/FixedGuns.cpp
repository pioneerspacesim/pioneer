#include "FixedGuns.h"

FixedGuns::FixedGuns()
{
}

FixedGuns::~FixedGuns()
{
}

bool FixedGuns::IsFiring()
{
	bool gunstate = false;
	for (int j = 0; j < FixedGuns::GUNMOUNT_MAX; j++)
		gunstate |= m_state[j];
	return gunstate;
}

void FixedGuns::Init(DynamicBody *b)
{
	for (int i=0; i<FixedGuns::GUNMOUNT_MAX; i++) {
		// Initialize structs
		m_state[i] = 0;
		m_gun[i].recharge = 0;
		m_gun[i].temp_slope = 0;
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

void FixedGuns::SaveToJson(int i, Json::Value &jsonObj )
{
	jsonObj["state"] = m_state[i];
	jsonObj["recharge"] = FloatToStr(m_recharge_stat[i]);
	jsonObj["temperature"] = FloatToStr(m_temperature_stat[i]);
};

void FixedGuns::LoadFromJson(int i, const Json::Value &jsonObj )
{
	m_state[i] = jsonObj["state"].asUInt();
	m_recharge_stat[i] = StrToFloat(jsonObj["recharge"].asString());
	m_temperature_stat[i] = StrToFloat(jsonObj["temperature"].asString());
};

void FixedGuns::InitGun( SceneGraph::Model *m, const char *tag, int num)
{
	const SceneGraph::MatrixTransform *mt = m->FindTagByName(tag);
	if (mt) {
		const matrix4x4f &trans = mt->GetTransform();
		m_gun[num].pos = trans.GetTranslate();
		m_gun[num].dir = trans.GetOrient().VectorZ();
	} else {
		// XXX deprecated
		m_gun[num].pos = (num==FixedGuns::GUN_FRONT) ? vector3f(0,0,0) : vector3f(0,0,0);
		m_gun[num].dir = (num==FixedGuns::GUN_REAR) ? vector3f(0,0,-1) : vector3f(0,0,1);
	}
}

void FixedGuns::MountGun(int num, float recharge, float lifespan, float damage, float length, float width, bool mining, const Color& color, float speed )
{
	if ( num >= FixedGuns::GUNMOUNT_MAX ) return;
	// Here we have projectile data MORE recharge time
	m_state[num] = 0;
	m_gun[num].recharge = recharge;
	m_gun[num].temp_slope = 0.01; // TODO: More fun if you have a variable "speed" for temperature
	m_gun[num].projData.lifespan = lifespan;
	m_gun[num].projData.damage = damage;
	m_gun[num].projData.length = length;
	m_gun[num].projData.width = width;
	m_gun[num].projData.mining = mining;
	m_gun[num].projData.color = color;
	m_gun[num].projData.speed = speed;
	m_gun_present[num] = true;
};

void FixedGuns::UnMountGun( int num )
{
		m_state[num] = 0;
		m_gun[num].recharge = 0;
		m_gun[num].temp_slope = 0;
		m_gun[num].projData.lifespan = 0;
		m_gun[num].projData.damage = 0;
		m_gun[num].projData.length = 0;
		m_gun[num].projData.width = 0;
		m_gun[num].projData.mining = false;
		m_gun[num].projData.speed = 0;
		m_gun[num].projData.color = Color::BLACK;
		m_gun_present[num] = false;
}

bool FixedGuns::Fire( int num, Body* b )
{
	if (!m_gun_present[num]) return false;
	if (!m_state[num]) return false;
	if (m_recharge_stat[num]>0) return false;
	if (m_temperature_stat[num] > 1.0) return false;
	const vector3d dir = b->GetOrient() * vector3d(m_gun[num].dir);
	const vector3d pos = b->GetOrient() * vector3d(m_gun[num].pos) + b->GetPosition();
	const vector3d dirVel = m_gun[num].projData.speed * dir.Normalized();

	m_temperature_stat[num] += m_gun[num].temp_slope;
	m_recharge_stat[num] = m_gun[num].recharge;

	if ( m_gun[num].dual )
	{
		const vector3d orient_norm = b->GetOrient().VectorY();
		const vector3d sep = 5.0 * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width, m_gun[num].projData.mining, m_gun[num].projData.color, pos + sep, b->GetVelocity(), dirVel);
		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width,m_gun[num].projData.mining, m_gun[num].projData.color, pos - sep, b->GetVelocity(), dirVel);
	} else {
		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width, m_gun[num].projData.mining, m_gun[num].projData.color, pos, b->GetVelocity(), dirVel);
	}
	return true;
};

void FixedGuns::UpdateGuns( float timeStep )
{
	for (int i=0; i<FixedGuns::GUNMOUNT_MAX; i++) {
		if ( !m_gun_present[i] ) continue;
		m_recharge_stat[i] -= timeStep;
		float rateCooling = m_gun[i].temp_slope;
		rateCooling *= m_cooler_boost;
		m_temperature_stat[i] -= rateCooling*timeStep;
		if (m_temperature_stat[i] < 0.0f) m_temperature_stat[i] = 0;
		if (m_recharge_stat[i] < 0.0f) m_recharge_stat[i] = 0;
	}

}
