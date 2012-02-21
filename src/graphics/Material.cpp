#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	diffuse(1.f), //default white
	unlit(false),
	twoSided(false),
	vertexColors(false),
	shader(0)
{
}

}
