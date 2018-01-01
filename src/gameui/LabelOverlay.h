// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_LABELOVERLAY_H
#define GAMEUI_LABELOVERLAY_H

#include "ui/Context.h"
#include "ui/Align.h"
#include "text/TextureFont.h"
#include "graphics/Frustum.h"
#include "graphics/Drawables.h"

namespace GameUI {

class LabelOverlay : public UI::Widget {
public:
	enum MarkerStyle { // <enum public name=GameUIMarkerStyle scope='GameUI::LabelOverlay' prefix=MARKER_>
		MARKER_NONE,
		MARKER_DOT,
	};

	struct Marker {
		std::string text;
		vector3f position;
		UI::Align::Direction textAnchor = UI::Align::TOP;
		Color4ub color = Color4ub(0, 255, 0, 255);
		MarkerStyle style = MARKER_DOT;
	};

	LabelOverlay(UI::Context *context);

	virtual UI::Point PreferredSize() override {
		return UI::Point(UI::Widget::SIZE_EXPAND);
	}

	virtual void Draw() override;

	Marker *AddMarker(const std::string &text, const vector3f &pos);
	void Clear();

	void SetView(const Graphics::Frustum &view_frustum);

private:
	void DrawMarker(const Marker &m, const vector2f &screen_pos);
	void DrawLabelText(const Marker &m, const vector2f &screen_pos);

	std::vector<std::unique_ptr<Marker>> m_markers;
	RefCountedPtr<Text::TextureFont> m_font;
	std::unique_ptr<Graphics::Drawables::TexturedQuad> m_markerDot;
	Graphics::Frustum m_view;
};

}

#endif
