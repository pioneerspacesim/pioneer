// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FresnelColourMaterial.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "TextureGL.h"
#include "RendererGL.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
namespace OGL {

FresnelColourProgram::FresnelColourProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;

	m_name = "FresnelColour";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

Program *FresnelColourMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new FresnelColourProgram(desc);
}

void FresnelColourMaterial::Apply()
{
	OGL::Material::Apply();
	FresnelColourProgram *p = static_cast<FresnelColourProgram*>(m_program);
	p->diffuse.Set(this->diffuse);
}

}
}
