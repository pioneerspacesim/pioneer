#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "libs.h"
#include <RefCounted.h>

namespace Graphics {

class Texture;
class Shader;

class MaterialDescriptor {
public:
	bool usePatterns; //colour customization system

	MaterialDescriptor() : usePatterns(false) { }

	friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b) {
		return (a.usePatterns == b.usePatterns);
	}
	friend bool operator<(const MaterialDescriptor &a, const MaterialDescriptor &b) {
		return (a.usePatterns != b.usePatterns);
	}
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
	virtual ~Material() { }

	Texture *texture0;
	Texture *texture1;
	Texture *texture2;
	Texture *texture3;
	Texture *texture4;
	Color diffuse;
	//Color ambient;
	Color specular;
	Color emissive;
	int shininess;
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

	bool newStyleHack;
	const MaterialDescriptor &GetDescriptor() const { return descriptor; }
	MaterialDescriptor descriptor;
};

}

#endif
