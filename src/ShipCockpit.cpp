// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipCockpit.h"
#include "ShipType.h"

ShipCockpit::ShipCockpit(const ShipType& ship_type) :
	m_type(ship_type)
{
	Init();
}

ShipCockpit::~ShipCockpit()
{

}

void ShipCockpit::Init()
{
	assert(!m_type.cockpitName.empty());
	SetModel(m_type.cockpitName.c_str());
	SetColliding(false);
	assert(GetModel());
}

void ShipCockpit::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	RenderModel(r, camera, viewCoords, viewTransform);
}