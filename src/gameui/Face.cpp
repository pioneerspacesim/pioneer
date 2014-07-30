// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "graphics/TextureBuilder.h"
#include "FaceGenManager.h"

using namespace UI;

namespace GameUI {

static const Uint32 FACE_WIDTH = 295;
static const Uint32 FACE_HEIGHT = 285;

RefCountedPtr<Graphics::Material> Face::s_material;

Face::Face(Context *context, Uint32 flags, Uint32 seed) : Single(context), m_preferredSize(INT_MAX)
{
	if (!seed) seed = time(0);

	m_flags = flags;
	m_seed = seed;

	SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FACE_WIDTH, FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

	Sint8 gender=0;
	FaceGenManager::BlitFaceIm(faceim, gender, flags, seed);

	m_texture.reset(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).CreateTexture(GetContext()->GetRenderer()));

	if (!s_material) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		s_material.Reset(GetContext()->GetRenderer()->CreateMaterial(matDesc));
	}

	m_preferredSize = UI::Point(FACE_WIDTH, FACE_HEIGHT);
	SetSizeControlFlags(UI::Widget::PRESERVE_ASPECT);
}

UI::Point Face::PreferredSize() {
	return m_preferredSize;
}

void Face::Layout()
{
	Point size(GetSize());
	Point activeArea(std::min(size.x, size.y));
	Point activeOffset(std::max(0, (size.x-activeArea.x)/2), std::max(0, (size.y-activeArea.y)/2));
	SetActiveArea(activeArea, activeOffset);

	Widget *innerWidget = GetInnerWidget();
	if (!innerWidget) return;
	SetWidgetDimensions(innerWidget, activeOffset, activeArea);
	innerWidget->Layout();
}

void Face::Draw()
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
	s_material->texture0 = m_texture.get();
	auto state = GetContext()->GetSkin().GetAlphaBlendState();
	r->DrawTriangles(&va, state, s_material.Get(), Graphics::TRIANGLE_STRIP);

	Single::Draw();
}

Face *Face::SetHeightLines(Uint32 lines)
{
	const Text::TextureFont *font = GetContext()->GetFont(GetFont()).Get();
	const float height = font->GetHeight() * lines;
	m_preferredSize = UI::Point(height * float(FACE_WIDTH) / float(FACE_HEIGHT), height);
	GetContext()->RequestLayout();
	return this;
}

}
