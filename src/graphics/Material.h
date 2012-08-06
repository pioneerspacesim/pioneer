#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "libs.h"
#include <RefCounted.h>

namespace Graphics {

class Texture;
class Shader;
class MaterialDescriptor;

enum EffectType {
	EFFECT_DEFAULT,
	EFFECT_STARFIELD,
	EFFECT_PLANETRING
};

/*
 * A generic material with some generic parameters.
 * Materials are created with Renderer::CreateMaterial
 */
class Material : public RefCounted {
public:
	Material();
	virtual ~Material() { }

	Texture *texture0;
	//Texture *texture1;
	Color diffuse;
	//Color ambient;
	//Color specular;
	Color emissive;
	//etc. Implement stuff when you need it, and also support
	//in renderers

	//this could be replaced with shade model: flat, phong etc.
	bool unlit;
	//in practice disables backface culling
	bool twoSided;
	// ignore material color and use vertex colors instead
	bool vertexColors;

	//custom glsl prog
	Shader *shader;

	virtual void Apply() { }
	virtual void Unapply() { }
};

// Renderer creates a material that best matches these requirements.
// EffectType may override some of the other flags.
class MaterialDescriptor {
public:
	MaterialDescriptor();
	EffectType effect;
	bool lighting;
	bool vertexColors;
	bool twoSided;
	int textures; //texture count
	int dirLights;

	friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b);
};

}

#endif
