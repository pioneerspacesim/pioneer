// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Light.h"

namespace Graphics {

	Light::Light() :
		m_type(LIGHT_POINT),
		m_position(0.f),
		m_intensity(1.0f),
		m_diffuse(Color::WHITE),
		m_specular(Color::BLANK)
	{
	}

	Light::Light(LightType t, const vector3f &p, const Color &d, const Color &s) :
		m_type(t),
		m_position(p),
		m_intensity(1.0f),
		m_diffuse(d),
		m_specular(s)
	{
	}

} // namespace Graphics
