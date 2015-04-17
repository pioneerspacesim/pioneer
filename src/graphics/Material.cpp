// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(nullptr),
	texture1(nullptr),
	texture2(nullptr),
	texture3(nullptr),
	texture4(nullptr),
	texture5(nullptr),
	heatGradient(nullptr),
	diffuse(Color::WHITE),
	specular(Color::BLACK),
	emissive(Color::BLACK),
	shininess(100), //somewhat sharp
	specialParameter0(nullptr)
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
, instanced(false)
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
		a.instanced == b.instanced &&
		a.textures == b.textures &&
		a.dirLights == b.dirLights &&
		a.quality == b.quality
	);
}

}
