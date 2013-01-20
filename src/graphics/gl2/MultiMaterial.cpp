// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MultiMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
namespace GL2 {

MultiProgram::MultiProgram(const MaterialDescriptor &desc, int lights)
{
	lights = Clamp(lights, 1, 4);

	//build some defines
	std::stringstream ss;
	if (desc.textures > 0)
		ss << "#define TEXTURE0\n";
	if (desc.vertexColors)
		ss << "#define VERTEXCOLOR\n";
	if (desc.alphaTest)
		ss << "#define ALPHA_TEST\n";
	//using only one light
	if (desc.lighting && lights > 0)
		ss << stringf("#define NUM_LIGHTS %0{d}\n", lights);
	else
		ss << "#define NUM_LIGHTS 0\n";

	if (desc.specularMap)
		ss << "#define MAP_SPECULAR\n";
	if (desc.glowMap)
		ss << "#define MAP_EMISSIVE\n";
	if (desc.usePatterns)
		ss << "#define MAP_COLOR\n";

	m_name = "multi";
	m_defines = ss.str();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

LitMultiMaterial::LitMultiMaterial()
: m_programs()
, m_curNumLights(0)
{
}

Program *MultiMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new MultiProgram(desc);
}

Program *LitMultiMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	m_curNumLights = m_renderer->m_numDirLights;
	return new MultiProgram(desc, m_curNumLights);
}

void LitMultiMaterial::SetProgram(Program *p)
{
	m_programs[m_curNumLights] = p;
	m_program = p;
}

void MultiMaterial::Apply()
{
	MultiProgram *p = static_cast<MultiProgram*>(m_program);
	p->Use();
	p->invLogZfarPlus1.Set(m_renderer->m_invLogZfarPlus1);

	p->diffuse.Set(this->diffuse);

	p->texture0.Set(this->texture0, 0);
	p->texture1.Set(this->texture1, 1);
	p->texture2.Set(this->texture2, 2);
	p->texture3.Set(this->texture3, 3);
	p->texture4.Set(this->texture4, 4);

	glPushAttrib(GL_ENABLE_BIT);
	if (this->twoSided)
		glDisable(GL_CULL_FACE);
}

void LitMultiMaterial::Apply()
{
	//request a new light variation
	if (m_curNumLights != m_renderer->m_numDirLights) {
		m_curNumLights = m_renderer->m_numDirLights;
		if (m_programs[m_curNumLights] == 0) {
			m_descriptor.dirLights = m_curNumLights; //hax
			m_programs[m_curNumLights] = m_renderer->GetOrCreateProgram(this);
		}
		m_program = m_programs[m_curNumLights];
	}

	MultiMaterial::Apply();

	MultiProgram *p = static_cast<MultiProgram*>(m_program);
	p->emission.Set(this->emissive);
	p->specular.Set(this->specular);
	p->shininess.Set(float(this->shininess));
	p->sceneAmbient.Set(m_renderer->GetAmbientColor());
}

void MultiMaterial::Unapply()
{
	glPopAttrib();
	//Might not be necessary to unbind textures, but let's not
	//confuse UI and LMR
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
