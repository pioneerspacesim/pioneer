// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CargoBody.h"
#include "Game.h"
#include "Pi.h"
#include "Serializer.h"
#include "Sfx.h"
#include "Space.h"
#include "EnumStrings.h"
#include "collider/collider.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"

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
	SetMassDistributionFromModel();

	std::vector<Color> colors;
	//metallic blue-orangeish color scheme
	colors.push_back(Color(255, 198, 64));
	colors.push_back(Color(0, 222, 255));
	colors.push_back(Color(255, 255, 255));

	SceneGraph::ModelSkin skin;
	skin.SetColors(colors);
	skin.SetDecal("pioneer");
	skin.Apply(GetModel());
	GetModel()->SetColors(colors);

	Properties().Set("type", EnumStrings::GetString("EquipType", m_type));
}

CargoBody::CargoBody(Equip::Type t)
{
	m_type = t;
	SetModel("cargo");
	Init();
	SetMass(1.0);
}

bool CargoBody::OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData)
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
	// ignore collision if its about to be scooped
	// XXX this is wrong. should only ignore if its actually going to be scooped. see Ship::OnCollision
	if (b->IsType(Object::SHIP)) {
		return true;
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

void CargoBody::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	RenderModel(r, camera, viewCoords, viewTransform);
}

void CargoBody::SetLabel(const std::string &label)
{
	assert(GetModel());
	GetModel()->SetLabel(label);
	Body::SetLabel(label);
}
