#include "Material.h"

Material::Material() :
	texture0(0),
	diffuse(1.f, 0.f, 1.f, 1.f),
	unlit(false),
	twoSided(false),
	vertexColors(false),
	shader(0)
{
}
