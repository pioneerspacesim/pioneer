#ifndef _MATERIAL_H
#define _MATERIAL_H
/*
 * Materials are used to apply an appropriate shader and other rendering parameters.
 * Users request materials from the renderer by filling a MaterialDescriptor structure,
 * and calling Renderer::CreateMaterial.
 * Users are responsible for deleting a material they have requested. This is because materials
 * are rarely shareable.
 * Material::Apply is called by renderer before drawing, and Unapply after drawing (to restore state).
 * For the GL2 renderer, a Material is always accompanied by a Program.
 */
#include "libs.h"
#include <RefCounted.h>

namespace Graphics {

class Texture;
class MaterialDescriptor;

// Shorthand for unique effects
// The other descriptor parameters may or may not have effect,
// depends on the effect
enum EffectType {
	EFFECT_DEFAULT,
	EFFECT_STARFIELD,
	EFFECT_PLANETRING,
	EFFECT_GEOSPHERE_TERRAIN,
	EFFECT_GEOSPHERE_SKY
};

/*
 * A generic material with some generic parameters.
 */
class Material : public RefCounted {
public:
	Material();
	virtual ~Material() { }

	Texture *texture0;
	//Texture *texture1;
	Color diffuse;
	//Color specular;
	Color emissive;
	//specular power etc.. implement things when you need them

	virtual void Apply() { }
	virtual void Unapply() { }

	//in practice disables backface culling
	bool twoSided;

	void *specialParameter0; //this can be whatever. Bit of a hack.
};

// Renderer creates a material that best matches these requirements.
// EffectType may override some of the other flags.
class MaterialDescriptor {
public:
	MaterialDescriptor();
	EffectType effect;
	bool atmosphere;
	bool lighting;
	bool vertexColors;
	bool twoSided;
	int textures; //texture count
	unsigned int dirLights; //set by rendererGL2 if lighting == true

	friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b);
};

}

#endif
