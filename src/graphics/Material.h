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
 * Materials define shading parameters. For example, when you
 * want to draw unlit geometry, define a material with the
 * 'unlit' flag set.
 * It is possible to override the shader choice with the
 * shader parameter (this is a hack, since Render::Shader is GL2 specific)
 */
//XXX I think renderer::RequestMaterial(...), renderer->RequestRenderTarget(...) style
//style approach would be better, but not doing that yet so objects don't need knowledge
//of renderer outside their Render() method.
class Material : public RefCounted {
public:
	Material();

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

	bool newStyleHack; //is this material created using RequestMaterial or not

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
	int texture; //texture0 type? texture count? don't know yet

	friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b);
};

}

#endif
