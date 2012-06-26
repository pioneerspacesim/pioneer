#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	texture1(0),
	texture2(0),
	texture3(0),
	texture4(0),
	diffuse(1.f), //default white
	specular(0.f),
	emissive(0.f),
	shininess(200.0),
	unlit(false),
	twoSided(false),
	vertexColors(false),
	blend(false),
	shader(0),
	newStyleHack(false)
{
}

}
