#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	diffuse(1.f), //default white
	emissive(Color::BLACK),
	unlit(false),
	twoSided(false),
	vertexColors(false)
{
}

MaterialDescriptor::MaterialDescriptor()
: effect(EFFECT_DEFAULT)
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
		a.lighting == b.lighting &&
		a.vertexColors == b.vertexColors &&
		a.twoSided == b.twoSided &&
		a.textures == b.textures &&
		a.dirLights == b.dirLights
	);
}

}
