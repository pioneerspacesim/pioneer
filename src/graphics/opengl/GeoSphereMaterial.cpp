// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoSphereMaterial.h"
#include "GeoSphere.h"
#include "Camera.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include <sstream>

namespace Graphics {
namespace OGL {

GeoSphereProgram::GeoSphereProgram(const std::string &filename, const std::string &defines)
{
	m_name = filename;
	m_defines = defines;
	LoadShaders(filename, defines);
	InitUniforms();
}

void GeoSphereProgram::InitUniforms()
{
	Program::InitUniforms();
	atmosColor.Init("atmosColor", m_program);
	geosphereAtmosFogDensity.Init("geosphereAtmosFogDensity", m_program);
	geosphereAtmosInvScaleHeight.Init("geosphereAtmosInvScaleHeight", m_program);
	geosphereAtmosTopRad.Init("geosphereAtmosTopRad", m_program);
	geosphereCenter.Init("geosphereCenter", m_program);
	geosphereScale.Init("geosphereScale", m_program);
	geosphereScaledRadius.Init("geosphereScaledRadius", m_program);

	shadows.Init("shadows", m_program);
	occultedLight.Init("occultedLight", m_program);
	shadowCentreX.Init("shadowCentreX", m_program);
	shadowCentreY.Init("shadowCentreY", m_program);
	shadowCentreZ.Init("shadowCentreZ", m_program);
	srad.Init("srad", m_program);
	lrad.Init("lrad", m_program);
	sdivlrad.Init("sdivlrad", m_program);
}

Program *GeoSphereSurfaceMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert((desc.effect == EFFECT_GEOSPHERE_TERRAIN) || 
		(desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA) ||
		(desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER));
	assert(desc.dirLights < 5);
	std::stringstream ss;
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	if(desc.dirLights>0) {
		const float invNumLights = 1.0f / float(desc.dirLights);
		ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
	}
	if (desc.quality & HAS_ATMOSPHERE)
		ss << "#define ATMOSPHERE\n";
	if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA)
		ss << "#define TERRAIN_WITH_LAVA\n";
	if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER)
		ss << "#define TERRAIN_WITH_WATER\n";
	if (desc.quality & HAS_ECLIPSES)
		ss << "#define ECLIPSE\n";
	return new Graphics::OGL::GeoSphereProgram("geosphere_terrain", ss.str());
}

void GeoSphereSurfaceMaterial::Apply()
{
	SetGSUniforms();
}

void GeoSphereSurfaceMaterial::SetGSUniforms()
{
	OGL::Material::Apply();

	GeoSphereProgram *p = static_cast<GeoSphereProgram*>(m_program);
	const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters*>(this->specialParameter0);
	const SystemBody::AtmosphereParameters ap = params.atmosphere;

	p->emission.Set(this->emissive);
	p->sceneAmbient.Set(m_renderer->GetAmbientColor());
	p->atmosColor.Set(ap.atmosCol);
	p->geosphereAtmosFogDensity.Set(ap.atmosDensity);
	p->geosphereAtmosInvScaleHeight.Set(ap.atmosInvScaleHeight);
	p->geosphereAtmosTopRad.Set(ap.atmosRadius);
	p->geosphereCenter.Set(ap.center);
	p->geosphereScaledRadius.Set(ap.planetRadius / ap.scale);
	p->geosphereScale.Set(ap.scale);

	//Light uniform parameters
	for( Uint32 i=0 ; i<m_renderer->GetNumLights() ; i++ ) {
		const Light& Light = m_renderer->GetLight(i);
		p->lights[i].diffuse.Set( Light.GetDiffuse() );
		p->lights[i].specular.Set( Light.GetSpecular() );
		const vector3f& pos = Light.GetPosition();
		p->lights[i].position.Set( pos.x, pos.y, pos.z, (Light.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f));
	}

	// we handle up to three shadows at a time
	int occultedLight[3] = {-1,-1,-1};
	vector3f shadowCentreX;
	vector3f shadowCentreY;
	vector3f shadowCentreZ;
	vector3f srad;
	vector3f lrad;
	vector3f sdivlrad;
	std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
	int j = 0;
	while (j<3 && it != itEnd) {
		occultedLight[j] = it->occultedLight;
		shadowCentreX[j] = it->centre[0];
		shadowCentreY[j] = it->centre[1];
		shadowCentreZ[j] = it->centre[2];
		srad[j] = it->srad;
		lrad[j] = it->lrad;
		sdivlrad[j] = it->srad / it->lrad;
		++it;
		++j;
	}
	p->shadows.Set(j);
	p->occultedLight.Set(occultedLight);
	p->shadowCentreX.Set(shadowCentreX);
	p->shadowCentreY.Set(shadowCentreY);
	p->shadowCentreZ.Set(shadowCentreZ);
	p->srad.Set(srad);
	p->lrad.Set(lrad);
	p->sdivlrad.Set(sdivlrad);
}

Program *GeoSphereSkyMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GEOSPHERE_SKY);
	assert(desc.dirLights > 0 && desc.dirLights < 5);
	std::stringstream ss;
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	if(desc.dirLights>0) {
		const float invNumLights = 1.0f / float(desc.dirLights);
		ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
	}
	ss << "#define ATMOSPHERE\n";
	if (desc.quality & HAS_ECLIPSES)
		ss << "#define ECLIPSE\n";
	return new Graphics::OGL::GeoSphereProgram("geosphere_sky", ss.str());
}

void GeoSphereSkyMaterial::Apply()
{
	SetGSUniforms();
}

}
}
