#include "CargoBody.h"
#include "Pi.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "Space.h"

static ObjParams params = {
	{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
};

void CargoBody::Save()
{
	using namespace Serializer::Write;
	DynamicBody::Save();
	wr_int(static_cast<int>(m_type));
	wr_float(m_hitpoints);
}

void CargoBody::Load()
{
	using namespace Serializer::Read;
	DynamicBody::Load();
	m_type = static_cast<Equip::Type>(rd_int());
	Init();
	m_hitpoints = rd_float();
}

void CargoBody::Init()
{
	m_hitpoints = 1.0f;
	SetLabel(EquipType::types[m_type].name);
	SetModel("cargo");
	SetMassDistributionFromModel();
}

CargoBody::CargoBody(Equip::Type t)
{
	m_type = t;
	Init();	
	SetMass(1.0);
}

bool CargoBody::OnDamage(Object *attacker, float kgDamage)
{
	m_hitpoints -= kgDamage*0.001f;
	if (m_hitpoints < 0) {
		Space::KillBody(this);
		Sfx::Add(this, Sfx::TYPE_EXPLOSION);
	}
	return true;
}

void CargoBody::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (!IsEnabled()) return;
	strncpy(params.pText[0], EquipType::types[m_type].name, 256);
	RenderSbreModel(viewCoords, viewTransform, &params);
}

