#include "FixedGuns.h"

FixedGuns::FixedGuns()
{
}

FixedGuns::~FixedGuns()
{
}

bool FixedGuns::IsFiring()
{
	// TODO: Faster way to know if ship is firing?
	bool gunstate = false;
	for (int j = 0; j < FixedGuns::GUNMOUNT_MAX; j++)
		gunstate |= m_gun[j].state;
	return gunstate;
}

void FixedGuns::Init(DynamicBody *b)
{
	for (int i=0; i<FixedGuns::GUNMOUNT_MAX; i++) {
		m_gun[i].state = 0;
		m_gun[i].recharge = 0;
		m_gun[i].temperature = 0;
		// XXX Here a "hack" to allow use of
		// guns be AI ships
		m_gun[i].projData.lifespan = 10;
		m_gun[i].projData.speed = 2000;
	}
	b->AddFeature( DynamicBody::FIXED_GUNS );
}

void FixedGuns::SaveToJson(int i, Json::Value &jsonObj )
{
	jsonObj["state"] = m_gun[i].state;
	jsonObj["recharge"] = FloatToStr(m_gun[i].recharge);
	jsonObj["temperature"] = FloatToStr(m_gun[i].temperature);
};

void FixedGuns::LoadFromJson(int i, const Json::Value &jsonObj )
{
	m_gun[i].state = jsonObj["state"].asUInt();
	m_gun[i].recharge = StrToFloat(jsonObj["recharge"].asString());
	m_gun[i].temperature = StrToFloat(jsonObj["temperature"].asString());
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

void FixedGuns::DefineGun(int num, float recharge, float lifespan, float damage, float length, float width, bool mining, const Color& color, float speed )
{
	if ( num >= FixedGuns::GUNMOUNT_MAX ) return;
	// Here we have projectile data MORE recharde time, this is a gun property
	m_gun[num].recharge = recharge;
	m_gun[num].projData.lifespan = lifespan;
	m_gun[num].projData.damage = damage;
	m_gun[num].projData.length = length;
	m_gun[num].projData.width = width;
	m_gun[num].projData.mining = mining;
	m_gun[num].projData.color = color;
	m_gun[num].projData.speed = speed;
};

bool FixedGuns::IsGunReady( int num )
{
	if ( !m_gun[num].state ) return false;
	if (m_gun[num].recharge > 0.0f) return false;
	if (m_gun[num].temperature > 1.0) return false;
	return true;
};

void FixedGuns::Fire( int num, Body* b, const matrix3x3d& shipOrient, const vector3d& shipVel, const vector3d& shipPos )
{

	const vector3d dir = shipOrient * vector3d(m_gun[num].dir);
	const vector3d pos = shipOrient * vector3d(m_gun[num].pos) + shipPos;
	const vector3d dirVel = m_gun[num].projData.speed * dir.Normalized();

	m_gun[num].temperature += 0.01f;

	if ( m_gun[num].dual )
	{
		const vector3d orient_norm = shipOrient.VectorY();
		const vector3d sep = 5.0 * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width, m_gun[num].projData.mining, m_gun[num].projData.color, pos + sep, shipVel, dirVel);
		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width,m_gun[num].projData.mining, m_gun[num].projData.color, pos - sep, shipVel, dirVel);
	} else {
		Projectile::Add( b, m_gun[num].projData.lifespan, m_gun[num].projData.damage, m_gun[num].projData.length, m_gun[num].projData.width, m_gun[num].projData.mining, m_gun[num].projData.color, pos, shipVel, dirVel);
	}

};

void FixedGuns::UpdateGuns( float timeStep )
{
	for (int i=0; i<FixedGuns::GUNMOUNT_MAX; i++) {
		m_gun[i].recharge -= timeStep;
		float rateCooling = 0.01f;
		rateCooling *= m_cooler_boost;
		m_gun[i].temperature -= rateCooling*timeStep;
		if (m_gun[i].temperature < 0.0f) m_gun[i].temperature = 0;
		if (m_gun[i].recharge < 0.0f) m_gun[i].recharge = 0;

		// Want fire gun?
		if (!m_gun[i].state) continue;
		// Ready to fire?
		if (m_gun[i].recharge > 0.0f) continue;
		// Too hot to fire?
		if (m_gun[i].temperature > 1.0) continue;
	}
}
