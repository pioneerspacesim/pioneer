// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Material.h"

namespace Graphics {

	Material::Material() :
		diffuse(Color::WHITE),
		specular(Color::BLACK),
		emissive(Color::BLACK),
		shininess(100.f) //somewhat sharp
	{
	}

	MaterialDescriptor::MaterialDescriptor() :
		effect(EFFECT_DEFAULT),
		alphaTest(false),
		glowMap(false),
		ambientMap(false),
		lighting(false),
		normalMap(false),
		specularMap(false),
		usePatterns(false),
		vertexColors(false),
		instanced(false),
		textures(0),
		dirLights(0),
		quality(0)
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
			a.normalMap == b.normalMap &&
			a.specularMap == b.specularMap &&
			a.usePatterns == b.usePatterns &&
			a.vertexColors == b.vertexColors &&
			a.instanced == b.instanced &&
			a.textures == b.textures &&
			a.dirLights == b.dirLights &&
			a.quality == b.quality);
	}

} // namespace Graphics
