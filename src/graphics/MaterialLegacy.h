#ifndef _GRAPHICS_MATERIALLEGACY_H
#define _GRAPHICS_MATERIALLEGAC_H
#include "graphics/Material.h"
/*
 * Legacy renderer materials. These are very simple.
 */
namespace Graphics {

class MaterialLegacy : public Material {
public:
	MaterialLegacy();

	virtual void Apply();
	virtual void Unapply();

	bool vertexColors;
	bool unlit;
};

class StarfieldMaterialLegacy : public MaterialLegacy {
public:
	virtual void Apply();
};

class GeoSphereSurfaceMaterialLegacy : public MaterialLegacy {
public:
	virtual void Apply();
	virtual void Unapply();
};

};

#endif
