#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	texture1(0),
	diffuse(1.f), //default white
	specular(0.f),
	unlit(false),
	twoSided(false),
	vertexColors(false),
	shader(0),
	newStyleHack(false)
{
}

}
