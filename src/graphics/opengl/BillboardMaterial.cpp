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
	return new Program("billboards", "");
}

void BillboardMaterial::Apply()
{
	OGL::Material::Apply();

	assert(this->texture0);
	static_cast<TextureGL*>(texture0)->Bind();
	m_program->texture0.Set(0);
}

void BillboardMaterial::Unapply()
{
	static_cast<TextureGL*>(texture0)->Unbind();
}

}
}
