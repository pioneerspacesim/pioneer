// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	texture1(0),
	texture2(0),
	texture3(0),
	texture4(0),
	texture5(0),
	heatGradient(0),
	diffuse(Color::WHITE),
	specular(Color::BLACK),
	emissive(Color::BLACK),
	shininess(100), //somewhat sharp
	specialParameter0(0)
{
}

MaterialDescriptor::MaterialDescriptor()
: effect(EFFECT_DEFAULT)
, alphaTest(false)
, glowMap(false)
, ambientMap(false)
, lighting(false)
, specularMap(false)
, usePatterns(false)
, vertexColors(false)
, textures(0)
, dirLights(0)
, quality(0)
{
}

bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b)
{
	return (
		a.effect == b.effect &&
		a.alphaTest == b.alphaTest &&
		a.glowMap == b.glowMap &&
		a.ambientMap == b.ambientMap &&
		a.lighting == b.lighting &&
		a.specularMap == b.specularMap &&
		a.usePatterns == b.usePatterns &&
		a.vertexColors == b.vertexColors &&
		a.textures == b.textures &&
		a.dirLights == b.dirLights &&
		a.quality == b.quality
	);
}

}
