// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ColorBackground.h"
#include "Context.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"

namespace UI {

ColorBackground::ColorBackground(Context *context, const Color &color) :
	Single(context)
{
	Graphics::MaterialDescriptor desc;
	m_material.Reset(GetContext()->GetRenderer()->CreateMaterial(desc));
	m_material->diffuse = color;
}

void ColorBackground::Draw()
{
	const Point &size = GetSize();

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION);
	va.Add(vector3f(0,      0,      0));
	va.Add(vector3f(0,      size.y, 0));
	va.Add(vector3f(size.x, 0,      0));
	va.Add(vector3f(size.x, size.y, 0));

	GetContext()->GetRenderer()->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);

	Container::Draw();
}

}
