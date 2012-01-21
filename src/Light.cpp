#include "Light.h"

Light::Light() :
	m_type(LIGHT_POINT),
	m_position(0.f),
	m_diffuse(Color(1.f)),
	m_ambient(Color(0.f)),
	m_specular(Color(0.f))
{

}

Light::Light(LightType t, const vector3f &p, const Color &d, const Color &a, const Color &s) :
	m_type(t),
	m_position(p),
	m_diffuse(d),
	m_ambient(a),
	m_specular(s)
{

}