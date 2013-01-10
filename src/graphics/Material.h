// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
class RendererLegacy;
class RendererGL2;

// Shorthand for unique effects
// The other descriptor parameters may or may not have effect,
// depends on the effect
enum EffectType {
	EFFECT_DEFAULT,
	EFFECT_STARFIELD,
	EFFECT_PLANETRING,
	EFFECT_GEOSPHERE_TERRAIN,
	EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA,
	EFFECT_GEOSPHERE_TERRAIN_WITH_WATER,
	EFFECT_GEOSPHERE_SKY
};

// Renderer creates a material that best matches these requirements.
// EffectType may override some of the other flags.
class MaterialDescriptor {
public:
	MaterialDescriptor();
	EffectType effect;
	bool alphaTest;
	bool atmosphere;
	bool glowMap;
	bool lighting;
	bool specularMap;
	bool twoSided;
	bool usePatterns; //pattern/color system
	bool vertexColors;
	int textures; //texture count
	unsigned int dirLights; //set by rendererGL2 if lighting == true

	friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b);
};

/*
 * A generic material with some generic parameters.
 */
class Material : public RefCounted {
public:
	Material();
	virtual ~Material() { }

	Texture *texture0;
	Texture *texture1;
	Texture *texture2;
	Texture *texture3;
	Texture *texture4;

	Color diffuse;
	Color specular;
	Color emissive;
	int shininess; //specular power 0-128

	virtual void Apply() { }
	virtual void Unapply() { }

	//in practice disables backface culling
	bool twoSided;

	void *specialParameter0; //this can be whatever. Bit of a hack.

	//XXX may not be necessary. Used by newmodel to check if a material uses patterns
	const MaterialDescriptor &GetDescriptor() const { return m_descriptor; }

protected:
	MaterialDescriptor m_descriptor;

private:
	friend class RendererLegacy;
	friend class RendererGL2;
};

}

#endif
