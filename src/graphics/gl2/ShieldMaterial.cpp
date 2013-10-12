// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShieldMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>
#include "StringF.h"
#include "Ship.h"

namespace Graphics {
namespace GL2 {

ShieldProgram::ShieldProgram(const MaterialDescriptor &desc, int lights)
{
	lights = Clamp(lights, 1, 4);

	//build some defines
	std::stringstream ss;

	m_name = "shield";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void ShieldProgram::InitUniforms()
{
	Program::InitUniforms();

	shieldStrength.Init("shieldStrength", m_program);
}

Program *ShieldMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new ShieldProgram(desc);
}

void ShieldMaterial::Apply()
{
	if(!this->specialParameter0)
		return;

	ShieldProgram *p = static_cast<ShieldProgram*>(m_program);
	const ShieldRenderParameters srp = *static_cast<ShieldRenderParameters*>(this->specialParameter0);
	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);

	p->diffuse.Set(this->diffuse);
	p->shieldStrength.Set(srp.strength);

	glPushAttrib(GL_ENABLE_BIT);
	if (this->twoSided)
		glDisable(GL_CULL_FACE);
}

void ShieldMaterial::Unapply()
{
	if(!this->specialParameter0)
		return;

	glPopAttrib();
	m_program->Unuse();
}

}
}
