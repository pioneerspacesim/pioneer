// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GuiTexturedQuad.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "GuiScreen.h"

using namespace Graphics;

namespace Gui {

void TexturedQuad::Draw(Graphics::Renderer *renderer, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint)
{
	PROFILE_SCOPED()

	// Create material on first use. Bit of a hack.
	if (!m_material) {
		PROFILE_SCOPED_RAW("!material")
		Graphics::MaterialDescriptor desc;
		desc.textures = 1;
		m_material.reset(renderer->CreateMaterial(desc));
		m_material->texture0 = m_texture.Get();
	}

	if(!m_vb.Get()) {
		PROFILE_SCOPED_RAW("!m_vb.get()")
		Graphics::VertexArray va(ATTRIB_POSITION | ATTRIB_UV0);

		// Size is always the same, modify it's position using the transform
		va.Add(vector3f(0.0f,		0.0f,      0.0f), vector2f(texPos.x,           texPos.y));
		va.Add(vector3f(0.0f,		0.0f+1.0f, 0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
		va.Add(vector3f(0.0f+1.0f,	0.0f,      0.0f), vector2f(texPos.x+texSize.x, texPos.y));
		va.Add(vector3f(0.0f+1.0f,	0.0f+1.0f, 0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));

		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_UV0;
		vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT2;
		vbd.numVertices = 4;
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		m_vb.Reset(renderer->CreateVertexBuffer(vbd));

		m_vb->Populate(va);
	}

	{
		// move and scale the quad on-screen
		Graphics::Renderer::MatrixTicket mt(renderer, Graphics::MatrixMode::MODELVIEW);
		const matrix4x4f& mv = renderer->GetCurrentModelView();
		matrix4x4f trans(matrix4x4f::Translation(vector3f(pos.x, pos.y, 0.0f)));
		trans.Scale(size.x, size.y, 0.0f);
		renderer->SetTransform(mv * trans);

		m_material->diffuse = tint;
		renderer->DrawBuffer(m_vb.Get(), Gui::Screen::alphaBlendState, m_material.get(), TRIANGLE_STRIP);
	}
}

}
