// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BillboardMaterial.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include "TextureGL.h"

namespace Graphics {
namespace OGL {

Program *BillboardMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.textures == 1);
	std::string defines;
	if(desc.effect == EFFECT_BILLBOARD_ATLAS)
		defines = stringf("#define USE_SPRITE_ATLAS\n");
	return new Program("billboards", defines);
}

void BillboardMaterial::Apply()
{
	OGL::Material::Apply();

	assert(this->texture0);
	m_program->texture0.Set(this->texture0, 0);
}

void BillboardMaterial::Unapply()
{
	static_cast<TextureGL*>(texture0)->Unbind();
}

}
}
