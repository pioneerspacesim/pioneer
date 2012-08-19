#include "GeoSphereMaterial.h"
#include "graphics/Graphics.h"
#include "GeoSphere.h"
#include "StringF.h"
#include <sstream>

namespace Graphics {
namespace GL2 {

GeoSphereProgram::GeoSphereProgram(const std::string &filename, const std::string &defines)
{
	LoadShaders(filename, defines);
	InitUniforms();
}

void GeoSphereProgram::SetUniforms(float radius, float scale,
	const vector3d &center, const SystemBody::AtmosphereParameters &ap)
{
	invLogZfarPlus1.Set(State::m_invLogZfarPlus1);
	atmosColor.Set(ap.atmosCol);
	geosphereAtmosFogDensity.Set(ap.atmosDensity);
	geosphereAtmosInvScaleHeight.Set(ap.atmosInvScaleHeight);
	geosphereAtmosTopRad.Set(ap.atmosRadius);
	geosphereCenter.Set(center);
	geosphereScaledRadius.Set(radius / scale);
	geosphereScale.Set(scale);
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
}

Program *GeoSphereSurfaceMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GEOSPHERE_TERRAIN);
	assert(desc.dirLights < 5);
	std::stringstream ss;
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	if (desc.atmosphere)
		ss << "#define ATMOSPHERE\n";
	return new Graphics::GL2::GeoSphereProgram("geosphere_terrain", ss.str());
}

void GeoSphereSurfaceMaterial::Apply()
{
	//XXX replace with actual material parameter
	glMaterialfv (GL_FRONT, GL_EMISSION, &emissive[0]);

	SetGSUniforms();
}

void GeoSphereSurfaceMaterial::SetGSUniforms()
{
	GeoSphereProgram *p = static_cast<GeoSphereProgram*>(m_program);
	const SystemBody::AtmosphereParameters ap = *static_cast<SystemBody::AtmosphereParameters*>(this->specialParameter0);

	p->Use();
	p->invLogZfarPlus1.Set(State::m_invLogZfarPlus1);
	p->atmosColor.Set(ap.atmosCol);
	p->geosphereAtmosFogDensity.Set(ap.atmosDensity);
	p->geosphereAtmosInvScaleHeight.Set(ap.atmosInvScaleHeight);
	p->geosphereAtmosTopRad.Set(ap.atmosRadius);
	p->geosphereCenter.Set(ap.center);
	p->geosphereScaledRadius.Set(ap.planetRadius / ap.scale);
	p->geosphereScale.Set(ap.scale);
}

Program *GeoSphereSkyMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_GEOSPHERE_SKY);
	assert(desc.dirLights > 0 && desc.dirLights < 5);
	std::stringstream ss;
	ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
	ss << "#define ATMOSPHERE\n";
	return new Graphics::GL2::GeoSphereProgram("geosphere_sky", ss.str());
}

void GeoSphereSkyMaterial::Apply()
{
	SetGSUniforms();
}

}
}
