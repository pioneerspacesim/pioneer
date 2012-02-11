#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "libs.h"
#include <RefCounted.h>

class Texture;
namespace Render {
	class Shader;
}

// Materials define shading and lighting parameters
// and shader choice for GL2 renderer
// As a hack (at this point) it is also possible to set a custom shader,
// set the uniforms yourself
class Material : public RefCounted {
public:
	Material();

	Texture *texture0;
	//Texture *texture1;
	Color diffuse;
	//Color ambient;
	//Color specular;

	//this could be replaced with shade model: flat, phong etc.
	bool unlit;
	//in practice disables backface culling
	bool twoSided;
	// ignore material color and use vertex colors instead
	bool vertexColors;

	//custom glsl prog
	Render::Shader *shader;
};

#endif
