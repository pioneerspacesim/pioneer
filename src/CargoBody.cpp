#include "CargoBody.h"
#include "Pi.h"
#include "ModelCollMeshData.h"
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
	{ "IR-L33T", "ME TOO" },
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
	SetModel(92);
	SetMassDistributionFromCollMesh(GetModelSBRECollMesh(92));
}

CargoBody::CargoBody(Equip::Type t)
{
	m_type = t;
	Init();	
	SetMass(1.0);
}

bool CargoBody::OnDamage(Body *attacker, float kgDamage)
{
	m_hitpoints -= kgDamage*0.001;
	if (m_hitpoints < 0) {
		Space::KillBody(this);
		Sfx::Add(this, Sfx::TYPE_EXPLOSION);
	}
	return true;
}

void CargoBody::Render(const Frame *camFrame)
{
	if (!IsEnabled()) return;
	RenderSbreModel(camFrame, 92, &params);
}

