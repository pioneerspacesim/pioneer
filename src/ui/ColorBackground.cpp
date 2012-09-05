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
	const vector2f &size = GetSize();

	// XXX I'd prefer a "clear region" method on the renderer, but I need to
	// look up what's possible at the GL/D3D level to support it

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION);
	va.Add(vector3f(0,      0,      0));
	va.Add(vector3f(0,      size.y, 0));
	va.Add(vector3f(size.x, 0,      0));
	va.Add(vector3f(size.x, size.y, 0));

	GetContext()->GetRenderer()->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);

	Container::Draw();
}

}
