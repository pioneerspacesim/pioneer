// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GuiTexturedQuad.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"

using namespace Graphics;

namespace Gui {

void TexturedQuad::Draw(Graphics::Renderer *renderer, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint)
{
	PROFILE_SCOPED()
    Graphics::VertexArray va(ATTRIB_POSITION | ATTRIB_UV0);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(texPos.x,           texPos.y));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(texPos.x+texSize.x, texPos.y));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));

	// Create material on first use. Bit of a hack.
	if (!m_material) {
		PROFILE_SCOPED_RAW("!material")
		Graphics::MaterialDescriptor desc;
		desc.textures = 1;
		m_material.reset(renderer->CreateMaterial(desc));
		m_material->texture0 = m_texture.Get();
	}
	m_material->diffuse = tint;
	renderer->DrawTriangles(&va, m_material.get(), TRIANGLE_STRIP);
}

}
