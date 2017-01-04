// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LabelOverlay.h"

namespace {

static vector2f label_offset(const GameUI::LabelOverlay::Marker &m) {
	if (m.style == GameUI::LabelOverlay::MARKER_NONE) {
		return vector2f(0.0f, 0.0f);
	}

	const float val = 15.0f;

	switch (m.textAnchor) {
		case UI::Align::TOP_LEFT:      return vector2f( val,  val);
		case UI::Align::TOP:           return vector2f(0.0f,  val);
		case UI::Align::TOP_RIGHT:     return vector2f(-val,  val);
		case UI::Align::LEFT:          return vector2f( val, 0.0f);
		case UI::Align::MIDDLE:        return vector2f(0.0f, 0.0f);
		case UI::Align::RIGHT:         return vector2f(-val, 0.0f);
		case UI::Align::BOTTOM_LEFT:   return vector2f( val, -val);
		case UI::Align::BOTTOM:        return vector2f(0.0f, -val);
		case UI::Align::BOTTOM_RIGHT:  return vector2f(-val, -val);
		default: assert(0 && "unpossible!"); return vector2f(0.0f, 0.0f);
	}
}

} // anonymous namespace

namespace GameUI {

LabelOverlay::LabelOverlay(UI::Context *context) : UI::Widget(context)
	, m_font(GetContext()->GetFont(GetFont()))
	, m_view(matrix4x4d::Identity(), matrix4x4d::Identity())
{
	SetSizeControlFlags(UI::Widget::NO_WIDTH | UI::Widget::NO_HEIGHT);
	m_markerDot.reset(new Graphics::Drawables::TexturedQuad(
		context->GetRenderer(), "icons/marker_dot.png"));
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

		vector2f screen_posf(screen_pos.x, screen_pos.y);
		DrawMarker(*m, screen_posf);
		DrawLabelText(*m, screen_posf);
	}
}

void LabelOverlay::DrawMarker(const Marker &m, const vector2f &screen_pos)
{
	if (m.style == MARKER_NONE) { return; }
	Graphics::Renderer *r = GetContext()->GetRenderer();
	Graphics::Renderer::MatrixTicket saveModelView(r, Graphics::MatrixMode::MODELVIEW);
	r->Translate(screen_pos.x, screen_pos.y, 0.0f);
	switch (m.style) {
		case MARKER_DOT:
			m_markerDot->Draw(r, m.color);
			break;
		default: assert(0 && "invalid marker style"); break;
	}
}

void LabelOverlay::DrawLabelText(const Marker &m, const vector2f &screen_pos)
{
	Graphics::Renderer *r = GetContext()->GetRenderer();
	Graphics::Renderer::MatrixTicket saveModelView(r, Graphics::MatrixMode::MODELVIEW);

	vector2f text_size, text_pos = screen_pos;

	m_font->MeasureString(m.text, text_size.x, text_size.y);

	switch (m.textAnchor) {
		case UI::Align::TOP_LEFT:
		case UI::Align::TOP:
		case UI::Align::TOP_RIGHT:
			break;
		case UI::Align::LEFT:
		case UI::Align::MIDDLE:
		case UI::Align::RIGHT:
			text_pos.y -= text_size.y*0.5f; break;
		case UI::Align::BOTTOM_LEFT:
		case UI::Align::BOTTOM:
		case UI::Align::BOTTOM_RIGHT:
			text_pos.y -= text_size.y; break;
	}

	switch (m.textAnchor) {
		case UI::Align::TOP_LEFT:
		case UI::Align::LEFT:
		case UI::Align::BOTTOM_LEFT:	break;
		case UI::Align::TOP:
		case UI::Align::MIDDLE:
		case UI::Align::BOTTOM:
			text_pos.x -= text_size.x*0.5f; break;
		case UI::Align::TOP_RIGHT:
		case UI::Align::RIGHT:
		case UI::Align::BOTTOM_RIGHT:
			text_pos.x -= text_size.x; break;
	}

	text_pos += label_offset(m);

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
			vb = m_font->CreateVertexBuffer(va, m.text, true);
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
