#include "GeoSphereMaterial.h"
#include "Graphics.h"

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

Program *GeoSphereSkyMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return 0;
}

}
}
