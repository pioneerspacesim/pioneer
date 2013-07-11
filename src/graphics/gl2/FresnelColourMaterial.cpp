// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FresnelColourMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>
#include "StringF.h"
#include "SectorView.h"

namespace Graphics {
namespace GL2 {

FresnelColourProgram::FresnelColourProgram(const MaterialDescriptor &desc, int lights)
{
	//build some defines
	std::stringstream ss;

	m_name = "FresnelColour";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void FresnelColourProgram::InitUniforms()
{
	Program::InitUniforms();
	
	fresnelCentre.Init("fresnelCentre", m_program);
}

Program *FresnelColourMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new FresnelColourProgram(desc);
}

void FresnelColourMaterial::Apply()
{
	FresnelColourProgram *p = static_cast<FresnelColourProgram*>(m_program);
	const TFresnelParams params = *static_cast<TFresnelParams*>(this->specialParameter0);

	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);

	p->diffuse.Set(this->diffuse);
	p->fresnelCentre.Set(params.m_centre);

	glPushAttrib(GL_ENABLE_BIT);
}

void FresnelColourMaterial::Unapply()
{
	glPopAttrib();
	m_program->Unuse();
}

}
}
