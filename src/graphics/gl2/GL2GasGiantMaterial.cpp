// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2GasGiantMaterial.h"
#include "GeoSphere.h"
#include "Camera.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "GL2Renderer.h"
#include <sstream>

namespace Graphics {
namespace GL2 {

// GasGiantProgram -------------------------------------------
GasGiantProgram::GasGiantProgram(const std::string &filename, const std::string &defines)
{
	m_name = filename;
	m_defines = defines;
	LoadShaders(filename, defines);
	InitUniforms();
}

void GasGiantProgram::InitUniforms()
{
	Program::InitUniforms();
	atmosColor.Init("atmosColor", m_program);
	geosphereAtmosFogDensity.Init("geosphereAtmosFogDensity", m_program);
	geosphereAtmosInvScaleHeight.Init("geosphereAtmosInvScaleHeight", m_program);
	geosphereAtmosTopRad.Init("geosphereAtmosTopRad", m_program);
	geosphereCenter.Init("geosphereCenter", m_program);
	geosphereRadius.Init("geosphereRadius", m_program);
	geosphereInvRadius.Init("geosphereInvRadius", m_program);

	shadowCentreX.Init("shadowCentreX", m_program);
	shadowCentreY.Init("shadowCentreY", m_program);
	shadowCentreZ.Init("shadowCentreZ", m_program);
	srad.Init("srad", m_program);
	lrad.Init("lrad", m_program);
	sdivlrad.Init("sdivlrad", m_program);
}

// GasGiantSurfaceMaterial -----------------------------------
GasGiantSurfaceMaterial::GasGiantSurfaceMaterial() : m_curNumShadows(0)
{
	for(int i=0;i<4;i++)
		m_programs[i] = nullptr;
}

Program *GasGiantSurfaceMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GASSPHERE_TERRAIN);
	assert(desc.dirLights < 5);
	std::stringstream ss;
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	if(desc.dirLights>0) {
		const float invNumLights = 1.0f / float(desc.dirLights);
		ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
	}
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";
	if (desc.quality & HAS_ATMOSPHERE)
		ss << "#define ATMOSPHERE\n";
	if (desc.quality & HAS_ECLIPSES)
		ss << "#define ECLIPSE\n";

	ss << stringf("#define NUM_SHADOWS %0{u}\n", m_curNumShadows);

	return new Graphics::GL2::GasGiantProgram("gassphere_base", ss.str());
}

void GasGiantSurfaceMaterial::SetProgram(Program *p)
{
	m_programs[m_curNumShadows] = p;
	m_program = p;
}

void GasGiantSurfaceMaterial::Apply()
{
	SwitchShadowVariant();
	SetGSUniforms();
}

void GasGiantSurfaceMaterial::SetGSUniforms()
{
	GL2::Material::Apply();

	GasGiantProgram *p = static_cast<GasGiantProgram*>(m_program);
	const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters*>(this->specialParameter0);
	const SystemBody::AtmosphereParameters ap = params.atmosphere;

	p->emission.Set(this->emissive);
	p->sceneAmbient.Set(m_renderer->GetAmbientColor());
	p->atmosColor.Set(ap.atmosCol);
	p->geosphereAtmosFogDensity.Set(ap.atmosDensity);
	p->geosphereAtmosInvScaleHeight.Set(ap.atmosInvScaleHeight);
	p->geosphereAtmosTopRad.Set(ap.atmosRadius);
	p->geosphereCenter.Set(ap.center);
	p->geosphereRadius.Set(ap.planetRadius);
	p->geosphereInvRadius.Set(1.0f / ap.planetRadius);

	//Light uniform parameters
	for( Uint32 i=0 ; i<m_renderer->GetNumLights() ; i++ ) {
		const Light& Light = m_renderer->GetLight(i);
		p->lights[i].diffuse.Set( Light.GetDiffuse() );
		p->lights[i].specular.Set( Light.GetSpecular() );
		const vector3f& pos = Light.GetPosition();
		p->lights[i].position.Set( pos.x, pos.y, pos.z, (Light.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f));
	}

	p->diffuse.Set(this->diffuse);
	p->texture0.Set(this->texture0, 0);

	// we handle up to three shadows at a time
	vector3f shadowCentreX;
	vector3f shadowCentreY;
	vector3f shadowCentreZ;
	vector3f srad;
	vector3f lrad;
	vector3f sdivlrad;
	std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
	int j = 0;
	while (j<3 && it != itEnd) {
		shadowCentreX[j] = it->centre[0];
		shadowCentreY[j] = it->centre[1];
		shadowCentreZ[j] = it->centre[2];
		srad[j] = it->srad;
		lrad[j] = it->lrad;
		sdivlrad[j] = it->srad / it->lrad;
		++it;
		++j;
	}
	p->shadowCentreX.Set(shadowCentreX);
	p->shadowCentreY.Set(shadowCentreY);
	p->shadowCentreZ.Set(shadowCentreZ);
	p->srad.Set(srad);
	p->lrad.Set(lrad);
	p->sdivlrad.Set(sdivlrad);
}

void GasGiantSurfaceMaterial::SwitchShadowVariant()
{
	const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters*>(this->specialParameter0);
	std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
	//request a new shadow variation
	if (m_curNumShadows != params.shadows.size()) {
		m_curNumShadows = std::min(Uint32(params.shadows.size()), 4U);
		if (m_programs[m_curNumShadows] == nullptr) {
			m_descriptor.numShadows = m_curNumShadows; //hax - so that GetOrCreateProgram will create a NEW shader instead of reusing the existing one
			m_programs[m_curNumShadows] = m_renderer->GetOrCreateProgram(this);
		}
		m_program = m_programs[m_curNumShadows];
	}
}

}
}
