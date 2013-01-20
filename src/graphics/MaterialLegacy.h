// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
