#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	texture1(0),
	texture2(0),
	texture3(0),
	texture4(0),
	diffuse(1.f),
	specular(0.f),
	emissive(0.f),
	shininess(200.0),
	unlit(false),
	twoSided(false),
	vertexColors(false),
	shader(0),
	newStyleHack(false)
{
}

MaterialDescriptor::MaterialDescriptor()
: glowMap(false)
, specularMap(false)
, usePatterns(false)
, alphaTest(false)
{
}

bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b)
{
	return (
		a.usePatterns == b.usePatterns
		&& a.glowMap == b.glowMap
		&& a.specularMap == b.specularMap
		&& a.alphaTest == b.alphaTest
	);
}

}
