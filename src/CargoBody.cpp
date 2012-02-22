#include "CargoBody.h"
#include "Pi.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "Space.h"
#include "LmrModel.h"
#include "Game.h"

void CargoBody::Save(Serializer::Writer &wr, Space *space)
{
	DynamicBody::Save(wr, space);
	wr.Int32(static_cast<int>(m_type));
	wr.Float(m_hitpoints);
}

void CargoBody::Load(Serializer::Reader &rd, Space *space)
{
	DynamicBody::Load(rd, space);
	m_type = static_cast<Equip::Type>(rd.Int32());
	Init();
	m_hitpoints = rd.Float();
}

void CargoBody::Init()
{
	m_hitpoints = 1.0f;
	SetLabel(Equip::types[m_type].name);
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
		Pi::game->GetSpace()->KillBody(this);
		Sfx::Add(this, Sfx::TYPE_EXPLOSION);
	}
	return true;
}

bool CargoBody::OnCollision(Object *b, Uint32 flags, double relVel)
{
	if (b->IsType(Object::SHIP) && (flags & 0x100)) {
		return true;
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

void CargoBody::Render(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (!IsEnabled()) return;
	GetLmrObjParams().label = Equip::types[m_type].name;
	RenderLmrModel(viewCoords, viewTransform);
}
