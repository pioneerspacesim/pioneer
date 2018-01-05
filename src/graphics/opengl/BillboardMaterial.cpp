// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BillboardMaterial.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include "TextureGL.h"
#include <sstream>

namespace Graphics {
namespace OGL {

BillboardProgram::BillboardProgram(const MaterialDescriptor &desc)
{
	//build some defines
	std::stringstream ss;

	m_name = "billboards";
	if(desc.effect == EFFECT_BILLBOARD_ATLAS)
		m_defines = stringf("#define USE_SPRITE_ATLAS\n");

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

void BillboardProgram::InitUniforms()
{
	Program::InitUniforms();
	coordDownScale.Init("coordDownScale", m_program);
}

Program *BillboardMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.textures == 1);
	return new BillboardProgram(desc);
}

void BillboardMaterial::Apply()
{
	OGL::Material::Apply();

	BillboardProgram *p = static_cast<BillboardProgram*>(m_program);

	assert(this->texture0);
	p->texture0.Set(this->texture0, 0);
	if (p->coordDownScale.IsValid()) {
		if(this->specialParameter0) {
			const float coordDownScale = *static_cast<float*>(this->specialParameter0);
			p->coordDownScale.Set(coordDownScale);
		} else {
			p->coordDownScale.Set(0.5f);
		}
	}
}

void BillboardMaterial::Unapply()
{
	static_cast<TextureGL*>(texture0)->Unbind();
}

}
}
