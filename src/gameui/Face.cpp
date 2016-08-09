// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "graphics/TextureBuilder.h"
#include "FaceParts.h"

using namespace UI;

namespace GameUI {

RefCountedPtr<Graphics::Material> Face::s_material;

Face::Face(Context *context, Uint32 flags, Uint32 seed) : Single(context), m_preferredSize(INT_MAX)
{
	if (!seed) seed = time(0);

	m_flags = flags;
	m_seed = seed;

	SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FaceParts::FACE_WIDTH, FaceParts::FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

	FaceParts::FaceDescriptor face;
	switch (flags & GENDER_MASK) {
		case RAND: face.gender = -1; break;
		case MALE: face.gender = 0; break;
		case FEMALE: face.gender = 1; break;
		default: assert(0); break;
	}

	FaceParts::PickFaceParts(face, m_seed);
	FaceParts::BuildFaceImage(faceim.Get(), face, (flags & ARMOUR));

	m_texture.Reset(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).GetOrCreateTexture(GetContext()->GetRenderer(), std::string("face")));

	if (!s_material) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		s_material.Reset(GetContext()->GetRenderer()->CreateMaterial(matDesc));
	}

	m_preferredSize = UI::Point(FaceParts::FACE_WIDTH, FaceParts::FACE_HEIGHT);
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
	Graphics::Renderer *r = GetContext()->GetRenderer();
	if (!m_quad) {
		const Point &offset = GetActiveOffset();
		const Point &area = GetActiveArea();

		const float x = offset.x;
		const float y = offset.y;
		const float sx = area.x;
		const float sy = area.y;

		const vector2f texSize = m_texture->GetDescriptor().texSize;

		Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
		va.Add(vector3f(x, y, 0.0f), vector2f(0.0f, 0.0f));
		va.Add(vector3f(x, y + sy, 0.0f), vector2f(0.0f, texSize.y));
		va.Add(vector3f(x + sx, y, 0.0f), vector2f(texSize.x, 0.0f));
		va.Add(vector3f(x + sx, y + sy, 0.0f), vector2f(texSize.x, texSize.y));

		s_material->texture0 = m_texture.Get();
		auto state = GetContext()->GetSkin().GetAlphaBlendState();
		m_quad.reset(new Graphics::Drawables::TexturedQuad(r, s_material, va, state));
	}
	m_quad->Draw(r);

	Single::Draw();
}

Face *Face::SetHeightLines(Uint32 lines)
{
	const Text::TextureFont *font = GetContext()->GetFont(GetFont()).Get();
	const float height = font->GetHeight() * lines;
	m_preferredSize = UI::Point(height * float(FaceParts::FACE_WIDTH) / float(FaceParts::FACE_HEIGHT), height);
	GetContext()->RequestLayout();
	return this;
}

}
