#include "Light.h"

Light::Light() :
	m_type(LIGHT_POINT),
	m_position(0.f),
	m_diffuse(Color(1.f)),
	m_ambient(Color(0.f)),
	m_specular(Color(0.f))
{

}