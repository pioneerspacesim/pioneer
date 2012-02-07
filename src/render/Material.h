#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <RefCounted.h>

//a bunch of renderstates and shaders are determined from this
//(can add shaderType or whatever hacks are necessary)
// Idea: to avoid if-else soup in Draw* functions, let renderers subclass Material
// with Apply() and perhaps Cleanup() methods. Users can then request
// materials with Material *mat = renderer->RequestMaterial(...)
class Material : public RefCounted {
public:
	Material();

	Texture *texture0;
	//Texture *texture1;
	Color diffuse;
	//Color ambient;
	//Color specular;
	bool unlit;
	bool twoSided;

	Render::Shader *shader; //custom glsl prog
	//etc
};

#endif
