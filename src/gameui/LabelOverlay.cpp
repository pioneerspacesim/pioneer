// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LabelOverlay.h"

namespace GameUI {

LabelOverlay::LabelOverlay(UI::Context *context) : UI::Widget(context)
	, m_font(GetContext()->GetFont(GetFont()))
	, m_view(matrix4x4d::Identity(), matrix4x4d::Identity())
{
	SetSizeControlFlags(UI::Widget::NO_WIDTH | UI::Widget::NO_HEIGHT);
}

void LabelOverlay::Draw()
{
	RefCountedPtr<Text::TextureFont> ui_font = GetContext()->GetFont(GetFont());
	if (m_font != ui_font) { m_font = ui_font; }

	UI::Point size = GetSize();
	const double sx = size.x;
	const double sy = size.y;

	for (const auto &m : m_markers) {
		vector3d screen_pos, text_pos;
		bool ok = m_view.ProjectPoint(vector3d(m->position), screen_pos);
		if (!ok) { continue; }
		screen_pos.x *= sx;
		screen_pos.y *= sy;
		const vector2f screen_posf(screen_pos.x, screen_pos.y);
		DrawLabelText(*m, screen_posf);
	}
}

void LabelOverlay::DrawLabelText(const Marker &m, const vector2f &screen_pos)
{
	Graphics::Renderer *r = GetContext()->GetRenderer();
	Graphics::Renderer::MatrixTicket saveModelView(r, Graphics::MatrixMode::MODELVIEW);

	vector2f text_size;
	m_font->MeasureString(m.text, text_size.x, text_size.y);

	const vector2f text_pos = screen_pos - text_size*0.5f;

	// Align to pixels (assumes that the rest of the transform stack doesn't
	// apply any scaling or rotation or non-integer translations...)
	r->Translate(std::floor(text_pos.x), std::floor(text_pos.y), 0.0f);
	auto *vb = m_font->GetCachedVertexBuffer(m.text);
	if (vb) {
		m_font->RenderBuffer(vb);
	} else {
		Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);
		m_font->PopulateString(va, m.text, 0.0f, 0.0f, Color::WHITE);
		if (va.GetNumVerts() > 0) {
			vb = m_font->CreateVertexBuffer(va, m.text);
			m_font->RenderBuffer(vb, m.color);
		}
	}
}

LabelOverlay::Marker *LabelOverlay::AddMarker(const std::string &text, const vector3f &pos)
{
	std::unique_ptr<Marker> m(new Marker);
	m->text = text;
	m->position = pos;
	Marker *mp = m.get();
	m_markers.push_back(std::move(m));
	return mp;
}

void LabelOverlay::Clear()
{
	m_markers.clear();
}

void LabelOverlay::SetView(const Graphics::Frustum &view_frustum)
{
	m_view = view_frustum;
}

}
