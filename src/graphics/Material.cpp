#include "Material.h"

namespace Graphics {

Material::Material() :
	texture0(0),
	diffuse(0.8f,0.8f,0.8f,1.0f),   // OpenGL default material
	ambient(0.2f,0.2f,0.2f,1.0f),
	specular(0.0f,0.0f,0.0f,1.0f),
	emissive(0.0f,0.0f,0.0f,1.0f),
	shininess(0.0f),
	unlit(false),
	twoSided(false),
	vertexColors(false),
	shader(0)
{
}

}
