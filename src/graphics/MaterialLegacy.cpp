// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MaterialLegacy.h"
#include "TextureGL.h"

namespace Graphics {

MaterialLegacy::MaterialLegacy()
: Material()
, vertexColors(false)
{
}

void MaterialLegacy::Apply()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

	if (!vertexColors)
		glColor4f(diffuse.r, diffuse.g, diffuse.b, diffuse.a);

	if (unlit) {
		glDisable(GL_LIGHTING);
	} else {
		glEnable(GL_LIGHTING);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, &diffuse[0]);
		//todo: the rest
	}
	if (twoSided) {
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glDisable(GL_CULL_FACE);
	}

	if (GetDescriptor().alphaTest)
		glEnable(GL_ALPHA_TEST);

	if (texture0)
		static_cast<TextureGL*>(texture0)->Bind();
}

void MaterialLegacy::Unapply()
{
	glPopAttrib();
	if (texture0)
		static_cast<TextureGL*>(texture0)->Unbind();
}

void StarfieldMaterialLegacy::Apply()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
	glDisable(GL_POINT_SMOOTH); //too large if smoothing is on
	glPointSize(1.0f);
	glDisable(GL_LIGHTING);
}

void GeoSphereSurfaceMaterialLegacy::Apply()
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv (GL_FRONT, GL_SPECULAR, &Color::BLACK[0]);
	glMaterialfv (GL_FRONT, GL_EMISSION, &emissive[0]);
}

void GeoSphereSurfaceMaterialLegacy::Unapply()
{
	glPopAttrib(); //lighting, color_material, normalize
}

}
