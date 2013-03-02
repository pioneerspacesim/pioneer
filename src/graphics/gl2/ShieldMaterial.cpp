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
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";
	if (desc.vertexColors)
		ss << "#define VERTEXCOLOR\n";

	assert(!desc.alphaTest);
	ss << "#define NUM_LIGHTS 0\n";

	if (desc.specularMap)
		ss << "#define MAP_SPECULAR\n";
	if (desc.glowMap)
		ss << "#define MAP_EMISSIVE\n";
	if (desc.usePatterns)
		ss << "#define MAP_COLOR\n";

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
	ShieldProgram *p = static_cast<ShieldProgram*>(m_program);
	const ShieldRenderParameters srp = *static_cast<ShieldRenderParameters*>(this->specialParameter0);
	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);

	p->diffuse.Set(this->diffuse);

	p->texture0.Set(this->texture0, 0);
	p->texture1.Set(this->texture1, 1);
	p->texture2.Set(this->texture2, 2);
	p->texture3.Set(this->texture3, 3);
	p->texture4.Set(this->texture4, 4);

	p->shieldStrength.Set(srp.strength);

	glPushAttrib(GL_ENABLE_BIT);
	if (this->twoSided)
		glDisable(GL_CULL_FACE);
}

void ShieldMaterial::Unapply()
{
	glPopAttrib();
	// Might not be necessary to unbind textures, but let's not old graphics code (eg, old-UI)
	if (texture4) {
		static_cast<TextureGL*>(texture4)->Unbind();
		glActiveTexture(GL_TEXTURE3);
	}
	if (texture3) {
		static_cast<TextureGL*>(texture3)->Unbind();
		glActiveTexture(GL_TEXTURE2);
	}
	if (texture2) {
		static_cast<TextureGL*>(texture2)->Unbind();
		glActiveTexture(GL_TEXTURE1);
	}
	if (texture1) {
		static_cast<TextureGL*>(texture1)->Unbind();
		glActiveTexture(GL_TEXTURE0);
	}
	if (texture0) {
		static_cast<TextureGL*>(texture0)->Unbind();
	}
	m_program->Unuse();
}

}
}
