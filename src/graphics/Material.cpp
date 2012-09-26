// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	diffuse(1.f), //default white
	emissive(Color::BLACK),
	twoSided(false),
	specialParameter0(0)
{
}

MaterialDescriptor::MaterialDescriptor()
: effect(EFFECT_DEFAULT)
, atmosphere(false)
, lighting(false)
, vertexColors(false)
, twoSided(false)
, textures(0)
, dirLights(0)
{
}

bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b)
{
	return (
		a.effect == b.effect &&
		a.atmosphere == b.atmosphere &&
		a.lighting == b.lighting &&
		a.vertexColors == b.vertexColors &&
		a.twoSided == b.twoSided &&
		a.textures == b.textures &&
		a.dirLights == b.dirLights
	);
}

}
