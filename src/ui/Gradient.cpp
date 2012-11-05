// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gradient.h"
#include "Context.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"

namespace UI {

Gradient::Gradient(Context *context, const Color &beginColor, const Color &endColor, Direction direction) :
	Single(context), m_beginColor(beginColor), m_endColor(endColor), m_direction(direction)
{
}

void Gradient::Update()
{
	// create texture and material before first use. a bit of a hack, but
	// probably better than allocating a texture in the constructor, where we
	// have no idea if we'll ever be drawn
	if (m_material.Valid()) return;

	Color4ub c0(m_beginColor);
	Color4ub c1(m_endColor);
	const unsigned char texData[4][4] = {
		{ c0.r, c0.g, c0.b, c0.a },
		{ c1.r, c1.g, c1.b, c1.a },
	};

	vector2f texSize = m_direction == HORIZONTAL ? vector2f(2.0f,1.0f) : vector2f(1.0f,2.0f);
	m_texture.Reset(GetContext()->GetRenderer()->CreateTexture(Graphics::TextureDescriptor(Graphics::TEXTURE_RGBA, texSize)));
	m_texture->Update(texData, texSize, Graphics::IMAGE_RGBA, Graphics::IMAGE_UNSIGNED_BYTE);

	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	m_material.Reset(GetContext()->GetRenderer()->CreateMaterial(desc));
	m_material->texture0 = m_texture.Get();
}

void Gradient::Draw()
{
	const Point &offset = GetActiveOffset();
	const Point &area = GetActiveArea();

	const float x = offset.x;
	const float y = offset.y;
	const float sx = area.x;
	const float sy = area.y;

	const vector2f texSize = m_texture->GetDescriptor().texSize;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(x,    y,    0.0f), vector2f(0.0f,      0.0f));
	va.Add(vector3f(x,    y+sy, 0.0f), vector2f(0.0f,      texSize.y));
	va.Add(vector3f(x+sx, y,    0.0f), vector2f(texSize.x, 0.0f));
	va.Add(vector3f(x+sx, y+sy, 0.0f), vector2f(texSize.x, texSize.y));

	Graphics::Renderer *r = GetContext()->GetRenderer();
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	r->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);

	Container::Draw();
}

}
